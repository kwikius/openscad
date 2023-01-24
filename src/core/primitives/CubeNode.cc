
#include <cmath>


#include <geometry/Geometry.h>
#include <geometry/PolySet.h>

#include "CubeNode.h"
#include <core/Visitable_inline.h>

std::string CubeNode::toString() const
{
 std::ostringstream stream;
 stream << "cube(size = ["
        << x << ", "
        << y << ", "
        << z << "], center = "
        << (center ? "true" : "false") << ")";
 return stream.str();
}

const Geometry *CubeNode::createGeometry() const
{
  double x1, x2, y1, y2, z1, z2;
  if (this->center) {
    x1 = -this->x / 2;
    x2 = +this->x / 2;
    y1 = -this->y / 2;
    y2 = +this->y / 2;
    z1 = -this->z / 2;
    z2 = +this->z / 2;
  } else {
    x1 = y1 = z1 = 0;
    x2 = this->x;
    y2 = this->y;
    z2 = this->z;
  }
  auto p = new PolySet(3, true);
  p->append_poly(); // top
  p->append_vertex(x1, y1, z2);
  p->append_vertex(x2, y1, z2);
  p->append_vertex(x2, y2, z2);
  p->append_vertex(x1, y2, z2);

  p->append_poly(); // bottom
  p->append_vertex(x1, y2, z1);
  p->append_vertex(x2, y2, z1);
  p->append_vertex(x2, y1, z1);
  p->append_vertex(x1, y1, z1);

  p->append_poly(); // side1
  p->append_vertex(x1, y1, z1);
  p->append_vertex(x2, y1, z1);
  p->append_vertex(x2, y1, z2);
  p->append_vertex(x1, y1, z2);

  p->append_poly(); // side2
  p->append_vertex(x2, y1, z1);
  p->append_vertex(x2, y2, z1);
  p->append_vertex(x2, y2, z2);
  p->append_vertex(x2, y1, z2);

  p->append_poly(); // side3
  p->append_vertex(x2, y2, z1);
  p->append_vertex(x1, y2, z1);
  p->append_vertex(x1, y2, z2);
  p->append_vertex(x2, y2, z2);

  p->append_poly(); // side4
  p->append_vertex(x1, y2, z1);
  p->append_vertex(x1, y1, z1);
  p->append_vertex(x1, y1, z2);
  p->append_vertex(x1, y2, z2);

  return p;
}
