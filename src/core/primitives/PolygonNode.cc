
#include <sstream>

#include <core/Children.h>
#include <core/Value.h>
#include <core/Parameters.h>
#include <core/Arguments.h>
#include <core/ModuleInstantiation.h>
#include <core/Visitable_inline.h>

#include <geometry/Polygon2d.h>
#include <geometry/Geometry.h>

#include "primitives.h"
#include "PolygonNode.h"

const Geometry *PolygonNode::createGeometry() const
{
  auto p = new Polygon2d();
  if (this->params.paths.empty() && this->params.points.size() > 2) {
    Outline2d outline;
    for (const auto& point : this->params.points) {
      outline.vertices.emplace_back(point.x, point.y);
    }
    p->addOutline(outline);
  } else {
    for (const auto& path : this->params.paths) {
      Outline2d outline;
      for (const auto& index : path) {
        assert(index < this->params.points.size());
        const auto& point = this->params.points[index];
        outline.vertices.emplace_back(point.x, point.y);
      }
      p->addOutline(outline);
    }
  }
  if (p->outlines().size() > 0) {
    p->setConvexity(this->params.convexity);
  }
  return p;
}

std::string PolygonNode::toString() const
{
  std::ostringstream stream;
  stream << "polygon(points = [";
  bool firstPoint = true;
  for (const auto& point : this->params.points) {
    if (firstPoint) {
      firstPoint = false;
    } else {
      stream << ", ";
    }
    stream << "[" << point.x << ", " << point.y << "]";
  }
  stream << "], paths = ";
  if (this->params.paths.empty()) {
    stream << "undef";
  } else {
    stream << "[";
    bool firstPath = true;
    for (const auto& path : this->params.paths) {
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
  stream << ", convexity = " << this->params.convexity << ")";
  return stream.str();
}

namespace primitives{

   std::shared_ptr<AbstractNode>
   builtin_polygon(const ModuleInstantiation *inst, Arguments arguments, const Children& children)
   {

     if (!children.empty()) {
       LOG(message_group::Warning, inst->location(), arguments.documentRoot(),
           "module %1$s() does not support child modules", inst->name());
     }

     Parameters parameters = Parameters::parse(std::move(arguments), inst->location(),
       {"points", "paths", "convexity"});

     if (parameters["points"].type() != Value::Type::VECTOR) {
       LOG(message_group::Error, inst->location(), parameters.documentRoot(),
          "Unable to convert points = %1$s to a vector of coordinates",
             parameters["points"].toEchoStringNoThrow());
       return  std::make_shared<PolygonNode>(inst);
     }

     polygon_params_t polygon;

     auto const & pointsVect = parameters["points"].toVector();
     for (const Value& pointValue : pointsVect) {
       primitives::point2d point;
       if (!pointValue.getVec2(point.x, point.y) ||
           !std::isfinite(point.x) || !std::isfinite(point.y)
           ) {
         LOG(message_group::Error, inst->location(), parameters.documentRoot(),
           "Unable to convert points[%1$d] = %2$s to a vec2 of numbers",
               polygon.points.size(), pointValue.toEchoStringNoThrow());
         polygon.points.push_back({0, 0});
       } else {
         polygon.points.push_back(point);
       }
     }

     if (parameters["paths"].type() == Value::Type::VECTOR) {
       size_t pathIndex = 0;
       auto const & pathsVect = parameters["paths"].toVector();
       for (const Value& pathValue : pathsVect) {
         if (pathValue.type() != Value::Type::VECTOR) {
           LOG(message_group::Error, inst->location(), parameters.documentRoot(),
              "Unable to convert paths[%1$d] = %2$s to a vector of numbers",
               pathIndex, pathValue.toEchoStringNoThrow());
         } else {
           size_t pointIndexIdx = 0;
           std::vector<size_t> path;
           auto const & pathValueVect = pathValue.toVector();
           for (const Value& pointIndexValue : pathValueVect) {
             if (pointIndexValue.type() != Value::Type::NUMBER) {
               LOG(message_group::Error, inst->location(), parameters.documentRoot(),
                 "Unable to convert paths[%1$d][%2$d] = %3$s to a number",
                  pathIndex, pointIndexIdx, pointIndexValue.toEchoStringNoThrow());
             } else {
               auto pointIndex = (size_t)pointIndexValue.toDouble();
               if (pointIndex < polygon.points.size()) {
                 path.push_back(pointIndex);
               } else {
                 LOG(message_group::Warning, inst->location(), parameters.documentRoot(),
                   "Point index %1$d is out of bounds (from paths[%2$d][%3$d])",
                       pointIndex, pathIndex, pointIndexIdx);
               }
             }
             pointIndexIdx++;
           }
           polygon.paths.push_back(std::move(path));
         }
         pathIndex++;
       }
     } else if (parameters["paths"].type() != Value::Type::UNDEFINED) {
       LOG(message_group::Error, inst->location(), parameters.documentRoot(),
          "Unable to convert paths = %1$s to a vector of vector of point indices",
             parameters["paths"].toEchoStringNoThrow());
       return std::make_shared<PolygonNode>(inst);
     }
     polygon.convexity = std::max(static_cast<int>(parameters["convexity"].toDouble()),1);

     return  std::make_shared<PolygonNode>(inst,std::move(polygon));
   }
}


