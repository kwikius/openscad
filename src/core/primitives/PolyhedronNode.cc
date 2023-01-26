
#include <sstream>
#include <cassert>
#include <vector>
#include <cmath>

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
#include "PolyhedronNode.h"

std::string PolyhedronNode::toString() const
{
  std::ostringstream stream;
  stream << "polyhedron(points = [";
  bool firstPoint = true;
  for (const auto& point : this->params.points) {
    if (firstPoint) {
      firstPoint = false;
    } else {
      stream << ", ";
    }
    stream << "[" << point.x << ", " << point.y << ", " << point.z << "]";
  }
  stream << "], faces = [";
  bool firstFace = true;
  for (const auto& face : this->params.faces) {
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
  stream << "], convexity = " << this->params.convexity << ")";
  return stream.str();
}

const Geometry *PolyhedronNode::createGeometry() const
{
  auto p = new PolySet(3);
  p->setConvexity(this->params.convexity);
  for (const auto& face : this->params.faces) {
    p->append_poly();
    for (const auto& index : face) {
      assert(index < this->params.points.size());
      const auto& point = this->params.points[index];
      p->insert_vertex(point.x, point.y, point.z);
    }
  }
  return p;
}

namespace primitives{



   std::shared_ptr<AbstractNode>
   builtin_polyhedron(const ModuleInstantiation *inst, Arguments arguments, const Children& children)
   {
     if (!children.empty()) {
       LOG(message_group::Warning, inst->location(), arguments.documentRoot(),
           "module %1$s() does not support child modules", inst->name());
     }

     polyhedron_params_t polyhedron;

     Parameters parameters = Parameters::parse(std::move(arguments), inst->location(),
       {"points", "faces", "convexity"}, {"triangles"});

     if (!isVector(parameters["points"])) {
       LOG(message_group::Error, inst->location(), parameters.documentRoot(),
        "Unable to convert points = %1$s to a vector of coordinates",
           parameters["points"].toEchoStringNoThrow());
       return std::make_shared<PolyhedronNode>(inst, std::move(polyhedron));
     }

     auto const & pointsVect = parameters["points"].toVector();
     for (const Value& pointValue : pointsVect ) {
       primitives::point3d point;
       if (!pointValue.getVec3(point.x, point.y, point.z, 0.0) ||
           !std::isfinite(point.x) || !std::isfinite(point.y) || !std::isfinite(point.z)
           ) {
         LOG(message_group::Error, inst->location(), parameters.documentRoot(),
           "Unable to convert points[%1$d] = %2$s to a vec3 of numbers",
             polyhedron.points.size(), pointValue.toEchoStringNoThrow());
         polyhedron.points.push_back({0, 0, 0});
       } else {
         polyhedron.points.push_back(std::move(point));
       }
     }

     const Value *pFaceVect = nullptr;
     if ( isUndefined(parameters["faces"]) &&
           isDefined(parameters["triangles"]) ) {
       // backwards compatible
       LOG(message_group::Deprecated, inst->location(), parameters.documentRoot(),
         "polyhedron(triangles=[]) will be removed in future releases. Use polyhedron(faces=[]) instead.");
       pFaceVect = &parameters["triangles"];
     } else {
       pFaceVect = &parameters["faces"];
     }
     if (!isVector(*pFaceVect)) {
       LOG(message_group::Error, inst->location(), parameters.documentRoot(),
         "Unable to convert faces = %1$s to a vector of vector of point indices", pFaceVect->toEchoStringNoThrow());
       return std::make_shared<PolyhedronNode>(inst, std::move(polyhedron));
     }

     size_t faceIndex = 0;
     auto const & faceValues = pFaceVect->toVector();
     for (const Value& faceValue :faceValues) {
       if (!isVector(faceValue)) {
         LOG(message_group::Error, inst->location(), parameters.documentRoot(),
            "Unable to convert faces[%1$d] = %2$s to a vector of numbers",
                faceIndex, faceValue.toEchoStringNoThrow());
       } else {
         size_t pointIndexIdx = 0;
         std::vector<size_t> face;
         auto const & faceValueVect = faceValue.toVector();
         for (const Value& pointIndexValue : faceValueVect) {
           if (!isNumber(pointIndexValue)) {
             LOG(message_group::Error, inst->location(), parameters.documentRoot(),
                "Unable to convert faces[%1$d][%2$d] = %3$s to a number",
                    faceIndex, pointIndexIdx, pointIndexValue.toEchoStringNoThrow());
           } else {
             auto const pointIndex = (size_t)pointIndexValue.toDouble();
             if (pointIndex < polyhedron.points.size()) {
               face.push_back(pointIndex);
             } else {
               LOG(message_group::Warning, inst->location(), parameters.documentRoot(),
                  "Point index %1$d is out of bounds (from faces[%2$d][%3$d])",
                     pointIndex, faceIndex, pointIndexIdx);
             }
           }
           pointIndexIdx++;
         }
         if (face.size() >= 3) {
           polyhedron.faces.push_back(std::move(face));
         }
       }
       faceIndex++;
     }

     polyhedron.convexity = std::max(static_cast<int>(parameters["convexity"].toDouble()),1);

     return std::make_shared<PolyhedronNode>(
        inst, std::move(polyhedron)
     );
   }
}
