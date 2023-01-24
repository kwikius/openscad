

#include <geometry/Polygon2d.h>
#include <sstream>
#include <utils/degree_trig.h>
#include <geometry/Geometry.h>
#include <utils/calc.h>
#include "CircleNode.h"
#include "primitives.h"
#include <core/Visitable_inline.h>

const Geometry* CircleNode::createGeometry() const
{
   auto const fragments = Calc::get_fragments_from_r(this->r, this->fn, this->fs, this->fa);
   auto const r = this->r;
   VectorOfVector2d circle(fragments);
   std::ranges::generate(circle,
      [r,fragments,i=0] () mutable {
         double const phi = (360.0 * i++) / fragments;
         return VectorOfVector2d::value_type{r * cos_degrees(phi),r * sin_degrees(phi)};
      }
   );
   Outline2d outline;
   outline.vertices = std::move(circle);
   auto p = new Polygon2d();
   p->addOutline(outline);
   p->setSanitized(true);
   return p;
}

std::string CircleNode::toString() const
{
 std::ostringstream stream;
 stream << "circle"
        << "($fn = " << fn
        << ", $fa = " << fa
        << ", $fs = " << fs
        << ", r = " << r
        << ")";
 return stream.str();
}
