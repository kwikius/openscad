

#include <sstream>
#include <cassert>
#include <cmath>

#include <geometry/Geometry.h>
#include <utils/calc.h>
#include <geometry/PolySet.h>
#include "primitives.h"
#include "PolyhedronNode.h"
#include <core/Visitable_inline.h>

std::string PolyhedronNode::toString() const
{
  std::ostringstream stream;
  stream << "polyhedron(points = [";
  bool firstPoint = true;
  for (const auto& point : this->points) {
    if (firstPoint) {
      firstPoint = false;
    } else {
      stream << ", ";
    }
    stream << "[" << point.x << ", " << point.y << ", " << point.z << "]";
  }
  stream << "], faces = [";
  bool firstFace = true;
  for (const auto& face : this->faces) {
    if (firstFace) {
      firstFace = false;
    } else {
      stream << ", ";
    }
    stream << "[";
    bool firstIndex = true;
    for (const auto& index : face) {
      if (firstIndex) {
        firstIndex = false;
      } else {
        stream << ", ";
      }
      stream << index;
    }
    stream << "]";
  }
  stream << "], convexity = " << this->convexity << ")";
  return stream.str();
}

const Geometry *PolyhedronNode::createGeometry() const
{
  auto p = new PolySet(3);
  p->setConvexity(this->convexity);
  for (const auto& face : this->faces) {
    p->append_poly();
    for (const auto& index : face) {
      assert(index < this->points.size());
      const auto& point = points[index];
      p->insert_vertex(point.x, point.y, point.z);
    }
  }
  return p;
}
