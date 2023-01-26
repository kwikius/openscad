
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
#include "CylinderNode.h"

const Geometry *CylinderNode::createGeometry() const
{
  auto p = new PolySet(3, true);

  if(!OpenSCAD::rangeCheck){
     if ( !isPositiveFinite(this->params.h) ||
          !isNonNegativeFinite(this->params.r1) ||
          !isNonNegativeFinite(this->params.r2) ||
          (this->params.r1 == 0 && this->params.r2 == 0)
       ) {
       return p;
     }
  }

  auto const fragments =
     primitives::get_fragments_from_r(
       std::fmax(this->params.r1, this->params.r2), this->params.fp);

  auto const circle1 = primitives::generate_circle( this->params.r1, fragments);
  auto const & circle2 = (this->params.r2 == this->params.r1)
    ? circle1
    : primitives::generate_circle( this->params.r2, fragments);

  double const z1 = this->params.center ? -this->params.h / 2 : 0 ;
  double const z2 = z1 + this->params.h;

  for (int i = 0; i < fragments; ++i) {
    int j = (i + 1) % fragments;
    if (this->params.r1 == this->params.r2) {
      p->append_poly();
      p->insert_vertex(circle1[i].x, circle1[i].y, z1);
      p->insert_vertex(circle2[i].x, circle2[i].y, z2);
      p->insert_vertex(circle2[j].x, circle2[j].y, z2);
      p->insert_vertex(circle1[j].x, circle1[j].y, z1);
    } else {
      if (this->params.r1 > 0) {
        p->append_poly();
        p->insert_vertex(circle1[i].x, circle1[i].y, z1);
        p->insert_vertex(circle2[i].x, circle2[i].y, z2);
        p->insert_vertex(circle1[j].x, circle1[j].y, z1);
      }
      if (this->params.r2 > 0) {
        p->append_poly();
        p->insert_vertex(circle2[i].x, circle2[i].y, z2);
        p->insert_vertex(circle2[j].x, circle2[j].y, z2);
        p->insert_vertex(circle1[j].x, circle1[j].y, z1);
      }
    }
  }

  if (this->params.r1 > 0) {
    p->append_poly();
    for (int i = 0; i < fragments; ++i)
      p->insert_vertex(circle1[i].x, circle1[i].y, z1);
  }

  if (this->params.r2 > 0) {
    p->append_poly();
    for (int i = 0; i < fragments; ++i)
      p->append_vertex(circle2[i].x, circle2[i].y, z2);
  }

  return p;
}

std::string CylinderNode::toString() const
{
 std::ostringstream stream;
 stream << "cylinder"
        << "($fn = " << this->params.fp.fn
        << ", $fa = " << this->params.fp.fa
        << ", $fs = " << this->params.fp.fs
        << ", h = " << this->params.h
        << ", r1 = " << this->params.r1
        << ", r2 = " << this->params.r2
        << ", center = " << (this->params.center ? "true" : "false")
        << ")";
 return stream.str();
}

namespace primitives{
   std::shared_ptr<AbstractNode>
   builtin_cylinder(const ModuleInstantiation *inst, Arguments arguments, const Children& children)
   {
     if (!children.empty()) {
       LOG(message_group::Warning, inst->location(), arguments.documentRoot(),
           "module %1$s() does not support child modules", inst->name());
     }

     Parameters parameters = Parameters::parse(std::move(arguments), inst->location(),
       {"h", "r1", "r2", "center"}, {"r", "d", "d1", "d2"});

     primitives::cylinder_params_t cyl;

     primitives::set_fragments(parameters, inst, cyl.fp);
     if (isNumber(parameters["h"])) {
         cyl.h = parameters["h"].toDouble();
     }

     auto r = primitives::lookup_radius(parameters, inst, "d", "r");
     auto r1 = primitives::lookup_radius(parameters, inst, "d1", "r1");
     auto r2 = primitives::lookup_radius(parameters, inst, "d2", "r2");
     if (isNumber(r) && ( isNumber(r1) || isNumber(r2))) {
       LOG(message_group::Warning, inst->location(), parameters.documentRoot(),
         "Cylinder parameters ambiguous");
     }

     if (isNumber(r)){
       cyl.r1 = r.toDouble();
       cyl.r2 = r.toDouble();
     }
     if (isNumber(r1)) {
       cyl.r1 = r1.toDouble();
     }
     if (isNumber(r2)) {
       cyl.r2 = r2.toDouble();
     }

     if (OpenSCAD::rangeCheck) {
        if (!isPositiveFinite(cyl.h)) {
            LOG(message_group::Warning, inst->location(), parameters.documentRoot(),
               "cylinder(h=%1$s, ...)", parameters["h"].toEchoStringNoThrow());
        }
        if (cyl.r1 < 0 || cyl.r2 < 0 ||
          (cyl.r1 == 0 && cyl.r2 == 0) ||
          !std::isfinite(cyl.r1) || !std::isfinite(cyl.r2)) {
            LOG(message_group::Warning, inst->location(), parameters.documentRoot(),
           "cylinder(r1=%1$s, r2=%2$s, ...)",
           (isNumber(r1) ? r1.toEchoStringNoThrow() : r.toEchoStringNoThrow()),
           (isNumber(r2) ? r2.toEchoStringNoThrow() : r.toEchoStringNoThrow()));
        }
     }
     if (isBool(parameters["center"])) {
        cyl.center = parameters["center"].toBool();
     }
     return  std::make_shared<CylinderNode>(inst,std::move(cyl));
   }
}
