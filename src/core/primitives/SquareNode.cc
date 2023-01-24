
#include <sstream>
#include <geometry/Polygon2d.h>
#include "SquareNode.h"

#include <core/Visitable_inline.h>


const Geometry *SquareNode::createGeometry() const
{
  auto p = new Polygon2d();
  if (
    this->x <= 0 || !std::isfinite(this->x)
    || this->y <= 0 || !std::isfinite(this->y)
    ) {
    return p;
  }

  Vector2d v1(0, 0);
  Vector2d v2(this->x, this->y);
  if (this->center) {
    v1 -= Vector2d(this->x / 2, this->y / 2);
    v2 -= Vector2d(this->x / 2, this->y / 2);
  }

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
        << x << ", "
        << y << "], center = "
        << (center ? "true" : "false") << ")";
 return stream.str();
}
