
#include <cassert>
#include <cmath>
#include <vector>
#include <sstream>

#include <utils/degree_trig.h>
#include <utils/calc.h>

#include <core/Children.h>
#include <core/Value.h>
#include <core/Parameters.h>
#include <core/Arguments.h>
#include <core/ModuleInstantiation.h>
#include <core/Visitable_inline.h>

#include <geometry/Geometry.h>
#include <geometry/Polygon2d.h>
#include <geometry/PolySet.h>

#include "primitives.h"
#include "CircleNode.h"

const Geometry* CircleNode::createGeometry() const
{
   auto p = new Polygon2d();
   // TODO should be invariant
   if (!OpenSCAD::rangeCheck && !isPositiveFinite(this->params.r)){
      // TODO add warning
      return p;
   }
   auto const fragments =
      primitives::get_fragments_from_r( this->params.r, this->params.fp);
   auto const r = this->params.r;
   VectorOfVector2d circle(fragments);
   std::ranges::generate(circle,
      [r,fragments,i=0] () mutable {
         double const phi = (360.0 * i++) / fragments;
         return VectorOfVector2d::value_type{r * cos_degrees(phi),r * sin_degrees(phi)};
      }
   );
   Outline2d outline;
   outline.vertices = std::move(circle);

   p->addOutline(outline);
   p->setSanitized(true);
   return p;
}

std::string CircleNode::toString() const
{
 std::ostringstream stream;
 stream << "circle"
        << "($fn = " << this->params.fp.fn
        << ", $fa = " << this->params.fp.fa
        << ", $fs = " << this->params.fp.fs
        << ", r = " << this->params.r
        << ")";
 return stream.str();
}

namespace primitives{

   std::shared_ptr<AbstractNode>
   builtin_circle(const ModuleInstantiation *inst, Arguments arguments, const Children& children)
   {
      if (!children.empty()) {
         LOG(message_group::Warning, inst->location(), arguments.documentRoot(),
         "module %1$s() does not support child modules", inst->name());
      }

      Parameters parameters = Parameters::parse(std::move(arguments), inst->location(), {"r"}, {"d"});

      primitives::circle_params_t circle;

      primitives::set_fragments(parameters, inst, circle.fp);
      const auto rv = primitives::lookup_radius(parameters, inst, "d", "r");
      if (isNumber(rv)) {
         // TODO RangeCheck here not in the CircleNode
         // for now this is how Vanilla does it
         // but rangeCheckis set to false
         // Then warn and set r to 1
         circle.r = rv.toDouble();
         if ( OpenSCAD::rangeCheck && !isPositiveFinite(circle.r) ){
            LOG(message_group::Warning, inst->location(), parameters.documentRoot(),
            "circle(r=%1$s)", rv.toEchoStringNoThrow());
         }
      }
      return std::make_shared<CircleNode>(
         *inst, std::move(circle)
      );
   }
}
