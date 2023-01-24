

#include <sstream>
#include <cassert>
#include <cmath>

#include "SphereNode.h"
#include "primitives.h"

#include <geometry/Geometry.h>
#include <utils/calc.h>
#include <geometry/PolySet.h>
#include <core/Visitable_inline.h>

std::string SphereNode::toString() const
{
 std::ostringstream stream;
 stream << "sphere"
        << "($fn = " << fn
        << ", $fa = " << fa
        << ", $fs = " << fs
        << ", r = " << r
        << ")";
 return stream.str();
}

const Geometry *SphereNode::createGeometry() const
{
  double const circle_radius = this->r;
  int const fragments = Calc::get_fragments_from_r(this->r, this->fn, this->fs, this->fa);
  int const numRings = (fragments + 1) / 2;
  struct ring_t {
    std::vector<primitives::point2d> points;
    double z;
  };
  auto ring = std::vector<ring_t>(numRings);
  std::ranges::generate(ring,[circle_radius,numRings,fragments,idx=0] () mutable {
       double const phi = (180.0 * (idx++ + 0.5)) / numRings;
       double const ring_radius = circle_radius * sin_degrees(phi);
       double const z = circle_radius * cos_degrees(phi);
       return ring_t{primitives::generate_circle(ring_radius,fragments),z};
     }
  );
  auto p = new PolySet(3, true);
  p->append_poly();
  for (int i = 0; i < fragments; ++i){
    p->append_vertex(ring[0].points[i].x, ring[0].points[i].y, ring[0].z);
  }
  for (int i = 0; i < numRings - 1; ++i) {
    auto r1 = &ring[i];
    auto r2 = &ring[i + 1];
    int r1i = 0, r2i = 0;
    while (r1i < fragments || r2i < fragments) {
      if (r1i >= fragments) goto sphere_next_r2;
      if (r2i >= fragments) goto sphere_next_r1;
      if ((double)r1i / fragments < (double)r2i / fragments) {
sphere_next_r1:
        p->append_poly();
        int r1j = (r1i + 1) % fragments;
        p->insert_vertex(r1->points[r1i].x, r1->points[r1i].y, r1->z);
        p->insert_vertex(r1->points[r1j].x, r1->points[r1j].y, r1->z);
        p->insert_vertex(r2->points[r2i % fragments].x, r2->points[r2i % fragments].y, r2->z);
        r1i++;
      } else {
sphere_next_r2:
        p->append_poly();
        int r2j = (r2i + 1) % fragments;
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


