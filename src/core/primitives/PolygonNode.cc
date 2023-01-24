

#include <geometry/Polygon2d.h>
#include <sstream>
#include <geometry/Geometry.h>
#include "PolygonNode.h"
#include <core/Visitable_inline.h>

const Geometry *PolygonNode::createGeometry() const
{
  auto p = new Polygon2d();
  if (this->paths.empty() && this->points.size() > 2) {
    Outline2d outline;
    for (const auto& point : this->points) {
      outline.vertices.emplace_back(point.x, point.y);
    }
    p->addOutline(outline);
  } else {
    for (const auto& path : this->paths) {
      Outline2d outline;
      for (const auto& index : path) {
        assert(index < this->points.size());
        const auto& point = points[index];
        outline.vertices.emplace_back(point.x, point.y);
      }
      p->addOutline(outline);
    }
  }
  if (p->outlines().size() > 0) {
    p->setConvexity(convexity);
  }
  return p;
}

std::string PolygonNode::toString() const
{
  std::ostringstream stream;
  stream << "polygon(points = [";
  bool firstPoint = true;
  for (const auto& point : this->points) {
    if (firstPoint) {
      firstPoint = false;
    } else {
      stream << ", ";
    }
    stream << "[" << point.x << ", " << point.y << "]";
  }
  stream << "], paths = ";
  if (this->paths.empty()) {
    stream << "undef";
  } else {
    stream << "[";
    bool firstPath = true;
    for (const auto& path : this->paths) {
      if (firstPath) {
        firstPath = false;
      } else {
        stream << ", ";
      }
      stream << "[";
      bool firstIndex = true;
      for (const auto& index : path) {
        if (firstIndex) {
          firstIndex = false;
        } else {
          stream << ", ";
        }
        stream << index;
      }
      stream << "]";
    }
    stream << "]";
  }
  stream << ", convexity = " << this->convexity << ")";
  return stream.str();
}


