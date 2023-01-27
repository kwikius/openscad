
#include <sstream>
#include <cassert>
#include <cmath>
#include <vector>

#include <utils/calc.h>
#include <utils/degree_trig.h>

#include <core/Children.h>
#include <core/Value.h>
#include <core/Parameters.h>
#include <core/Arguments.h>
#include <core/ModuleInstantiation.h>
#include <core/Visitable_inline.h>

#include <geometry/Polygon2d.h>
#include <geometry/PolySet.h>

#include "primitives.h"
#include "SquareNode.h"

const Geometry *SquareNode::createGeometry() const
{
  auto p = new Polygon2d();
  if ( !OpenSCAD::rangeCheck && ! isPositiveFinite(this->params.size) ) {
    return p;
  }
  using primitives::point2d;

  point2d const p1 = (this->params.center)
     ? -this->params.size/2
     : point2d{0.0,0.0};
  point2d const p2 = p1 + this->params.size;
  Vector2d const v1(p1.x,p1.y);
  Vector2d const v2(p2.x,p2.y);

  Outline2d o;
  o.vertices = {v1, {v2[0], v1[1]}, v2, {v1[0], v2[1]}};

  p->addOutline(o);
  p->setSanitized(true);
  return p;
}

std::string SquareNode::toString() const
{
 std::ostringstream stream;
 stream << "square(size = ["
        << this->params.size.x << ", "
        << this->params.size.y << "], center = "
        << (this->params.center ? "true" : "false") << ")";
 return stream.str();
}

namespace primitives{

   std::shared_ptr<AbstractNode>
   builtin_square(const ModuleInstantiation *inst, Arguments arguments, const Children& children)
   {
     if (!children.empty()) {
       LOG(message_group::Warning, inst->location(), arguments.documentRoot(),
           "module %1$s() does not support child modules", inst->name());
     }

     Parameters parameters = Parameters::parse(std::move(arguments), inst->location(), {"size", "center"});

     primitives::square_params_t square;

     const auto& size = parameters["size"];

     if (size.isDefined()) {
       bool converted = false;
       converted |= size.getDouble(square.size.x);
       converted |= size.getDouble(square.size.y);
       converted |= size.getVec2(square.size.x, square.size.y);
       if (!converted) {
         LOG(message_group::Warning, inst->location(), parameters.documentRoot(),
            "Unable to convert square(size=%1$s, ...) parameter to a number or a vec2 of numbers",
               size.toEchoStringNoThrow());
       }else if ( OpenSCAD::rangeCheck && ! isPositiveFinite(square.size) ) {
           LOG(message_group::Warning, inst->location(), parameters.documentRoot(),
             "square(size=%1$s, ...)", size.toEchoStringNoThrow());
       }
     }
     if (parameters["center"].type() == Value::Type::BOOL) {
       square.center = parameters["center"].toBool();
     }
     return std::make_shared<SquareNode>(*inst,std::move(square));
   }
}
