
#include <sstream>
#include <cassert>
#include <cmath>

#include "CylinderNode.h"
#include "primitives.h"

#include <geometry/Geometry.h>
#include <utils/calc.h>
#include <geometry/PolySet.h>
#include <core/Visitable_inline.h>

const Geometry *CylinderNode::createGeometry() const
{
  auto fragments = Calc::get_fragments_from_r(
    std::fmax(this->r1, this->r2), this->fn, this->fs, this->fa);

  auto const circle1 = primitives::generate_circle( this->r1, fragments);
  auto const circle2 = (this->r2 == this->r1)
    ? circle1
    : primitives::generate_circle( this->r2, fragments);

  double const z1 = this->center ? -this->h / 2 : 0 ;
  double const z2 = z1 + this->h;

  auto p = new PolySet(3, true);
  for (int i = 0; i < fragments; ++i) {
    int j = (i + 1) % fragments;
    if (r1 == r2) {
      p->append_poly();
      p->insert_vertex(circle1[i].x, circle1[i].y, z1);
      p->insert_vertex(circle2[i].x, circle2[i].y, z2);
      p->insert_vertex(circle2[j].x, circle2[j].y, z2);
      p->insert_vertex(circle1[j].x, circle1[j].y, z1);
    } else {
      if (r1 > 0) {
        p->append_poly();
        p->insert_vertex(circle1[i].x, circle1[i].y, z1);
        p->insert_vertex(circle2[i].x, circle2[i].y, z2);
        p->insert_vertex(circle1[j].x, circle1[j].y, z1);
      }
      if (r2 > 0) {
        p->append_poly();
        p->insert_vertex(circle2[i].x, circle2[i].y, z2);
        p->insert_vertex(circle2[j].x, circle2[j].y, z2);
        p->insert_vertex(circle1[j].x, circle1[j].y, z1);
      }
    }
  }

  if (this->r1 > 0) {
    p->append_poly();
    for (int i = 0; i < fragments; ++i)
      p->insert_vertex(circle1[i].x, circle1[i].y, z1);
  }

  if (this->r2 > 0) {
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
        << "($fn = " << fn
        << ", $fa = " << fa
        << ", $fs = " << fs
        << ", h = " << h
        << ", r1 = " << r1
        << ", r2 = " << r2
        << ", center = " << (center ? "true" : "false")
        << ")";
 return stream.str();
}
