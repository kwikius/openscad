

#include <sstream>
#include <cassert>
#include <cmath>

#include <utils/calc.h>

#include <core/Children.h>
#include <core/Value.h>
#include <core/Parameters.h>
#include <core/Arguments.h>
#include <core/ModuleInstantiation.h>
#include <core/Visitable_inline.h>

#include <geometry/Geometry.h>
#include <geometry/PolySet.h>

#include "primitives.h"
#include "SphereNode.h"

std::string SphereNode::toString() const
{
 std::ostringstream stream;
 stream << "sphere"
        << "($fn = " << this->params.fp.fn
        << ", $fa = " << this->params.fp.fa
        << ", $fs = " << this->params.fp.fs
        << ", r = " << this->params.r
        << ")";
 return stream.str();
}

const Geometry *SphereNode::createGeometry() const
{
  auto poly = new PolySet(3, true);

  if (!OpenSCAD::rangeCheck && !isPositiveFinite(this->params.r)) {
    return poly;
  }
  double const circle_radius = this->params.r;
  int const fragments =
     primitives::get_fragments_from_r(this->params.r,this->params.fp);
  int const numRings = (fragments + 1) / 2;
  struct ring_t {
    std::vector<primitives::point2d> points;
    double z;
  };
  // make a prototype ring
  auto const proto_ring = primitives::generate_circle(circle_radius,fragments);
  // and generate the sphere rings by scaling from the prototype
  auto ring = std::vector<ring_t>(numRings);
  std::ranges::generate(ring,[&proto_ring, circle_radius, numRings, i=0] () mutable{
       double const phi = (180.0 * (i++ + 0.5)) / numRings;
       double const scale = sin_degrees(phi);
       double const z = circle_radius * cos_degrees(phi);
       auto v = std::vector<primitives::point2d>(proto_ring.size());
       std::ranges::generate(v, [&proto_ring,scale,j = 0]() mutable{
          return proto_ring[j++] * scale;
       });
       return ring_t{std::move(v),z};
    }
  );
  using primitives::point2d;

  auto add_pt = [&poly](ring_t const & ri, int i)
  {
     auto const & pt = ri.points[i];
     poly->append_vertex(pt.x,pt.y,ri.z);
  };

  auto add_ring = [add_pt,&poly](ring_t const & ri)
  {
     int const size = ri.points.size();
     poly->append_poly();
     for (int i = 0; i < size;++i){
       add_pt(ri,i);
     }
  };

  add_ring(ring[0]);

  for (int i = 0; i < numRings - 1; ++i) {
    auto const & r1 = ring[i];
    auto const & r2 = ring[i + 1];
    int r1i = 0, r2i = 0;
    while (r1i < fragments || r2i < fragments) {
      if (r1i >= fragments) goto sphere_next_r2;
      if (r2i >= fragments) goto sphere_next_r1;
      if ((double)r1i / fragments < (double)r2i / fragments) {
sphere_next_r1:
        poly->append_poly();
        int const r1j = (r1i + 1) % fragments;
        add_pt(r1,r1i);
        add_pt(r1,r1j);
        add_pt(r2,r2i % fragments);
        ++r1i;
      } else {
sphere_next_r2:
        poly->append_poly();
        int const r2j = (r2i + 1) % fragments;
        add_pt(r2,r2i);
        add_pt(r2,r2j);
        add_pt(r1,r1i % fragments);
        ++r2i;
      }
    }
  }
  add_ring(ring[numRings - 1]);
  return poly;
}

namespace primitives{

   std::shared_ptr<AbstractNode>
   builtin_sphere(const ModuleInstantiation *inst, Arguments arguments, const Children& children)
   {
     if (!children.empty()) {
       LOG(message_group::Warning, inst->location(), arguments.documentRoot(),
           "module %1$s() does not support child modules", inst->name());
     }

     Parameters parameters = Parameters::parse(std::move(arguments), inst->location(), {"r"}, {"d"});
     primitives::sphere_params_t sphere;
     // set from The "global special variables"
     primitives::set_fragments(parameters, inst, sphere.fp);
     const auto sphereRadiusValue = primitives::lookup_radius(parameters, inst, "d", "r");

     if (isNumber(sphereRadiusValue)) {
       double const r = sphereRadiusValue.toDouble();
       if (OpenSCAD::rangeCheck && !isPositiveFinite(r)) {
         LOG(message_group::Warning, inst->location(), parameters.documentRoot(),
             "sphere(r=%1$s)", sphereRadiusValue.toEchoStringNoThrow());
       }else{
         sphere.r = r;
       }
     }// TODO else warn?
     return std::make_shared<SphereNode>(*inst, std::move(sphere));
   }
}
