

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
        << ", r = " << this->params.r;

   if (std::holds_alternative<bool>(this->params.center)){
      bool const center = std::get<bool>(this->params.center);
      if ( center == true){
         stream  << ")"; // default
      }else{
        stream << ", center = false)";
      }
   }else{
      using primitives::point3di;
      point3di center = std::get<point3di>(this->params.center);
      stream << ", center = " << "[" << center.x <<", "
        << center.y << ", " << center.z << "])";
   }
   return stream.str();
}

const Geometry *SphereNode::createGeometry() const
{
  auto p = new PolySet(3, true);

  if (!OpenSCAD::rangeCheck && !isPositiveFinite(this->params.r)) {
    return p;
  }
   using primitives::point3di;
   using primitives::point2di;
   using primitives::point3d;
   using primitives::point2d;

   point3di center;
   if ( std::holds_alternative<bool>(this->params.center)){
      bool b = std::get<bool>(this->params.center);
      center = point3di{0,0,b?0:1};
   }else{
      center = std::get<point3di>(this->params.center);
   }

  double const circle_radius = this->params.r;
  int const fragments =
     primitives::get_fragments_from_r(this->params.r,this->params.fp);

  int const numRings = (fragments + 1) / 2;
  struct ring_t {
    std::vector<point2d> points;
    double z;
  };

  auto const proto_ring = primitives::generate_circle(circle_radius,fragments);

  auto const offset = center * this->params.r;
  auto ring = std::vector<ring_t>(numRings);
  std::generate(ring.begin(),ring.end() ,
    [&proto_ring, circle_radius,numRings,offset,idx=0] () mutable{
       double const phi = (180.0 * (idx++ + 0.5)) / numRings;
       double const scale = sin_degrees(phi);
       auto v = std::vector<point2d>(proto_ring.size());
       std::ranges::generate(v, [&proto_ring,scale,offset,idx1 = 0]() mutable{
          return proto_ring[idx1++] * scale + point2di{offset.x,offset.y};
       });
       double const z = circle_radius * cos_degrees(phi) + offset.z;
       return ring_t{std::move(v),z};
    }
  );

  p->append_poly();
  for (int i = 0; i < fragments; ++i){
    p->append_vertex(ring[0].points[i].x, ring[0].points[i].y, ring[0].z);
  }
  for (int i = 0; i < numRings - 1; ++i) {
    auto r1 = &ring[i];
    auto r2 = &ring[i + 1];
    int r1i = 0, r2i = 0;
    while (r1i < fragments || r2i < fragments) {
      p->append_poly();
      if ( (r1i < fragments) && ( (r2i >= fragments) || (r1i < r2i) )){
        int const r1j = (r1i + 1) % fragments;
        p->insert_vertex(r1->points[r1i].x, r1->points[r1i].y, r1->z);
        p->insert_vertex(r1->points[r1j].x, r1->points[r1j].y, r1->z);
        p->insert_vertex(r2->points[r2i % fragments].x, r2->points[r2i % fragments].y, r2->z);
        r1i++;
      } else {
        int const r2j = (r2i + 1) % fragments;
        p->append_vertex(r2->points[r2i].x, r2->points[r2i].y, r2->z);
        p->append_vertex(r2->points[r2j].x, r2->points[r2j].y, r2->z);
        p->append_vertex(r1->points[r1i % fragments].x, r1->points[r1i % fragments].y, r1->z);
        r2i++;
      }
    }
  }

  p->append_poly();
  for (int i = 0; i < fragments; ++i) {
    p->insert_vertex(
      ring[numRings - 1].points[i].x,
      ring[numRings - 1].points[i].y,
      ring[numRings - 1].z
      );
  }
  return p;
}

namespace primitives{

   std::shared_ptr<AbstractNode>
   builtin_sphere(const ModuleInstantiation *inst, Arguments arguments, const Children& children)
   {
     if (!children.empty()) {
       LOG(message_group::Warning, inst->location(), arguments.documentRoot(),
           "module %1$s() does not support child modules", inst->name());
     }

     Parameters parameters = Parameters::parse(std::move(arguments), inst->location(), {"r","center"}, {"d"});
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
     primitives::get_center(parameters,sphere.center);
     return std::make_shared<SphereNode>(*inst, std::move(sphere));
   }
}
