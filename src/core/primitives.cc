/*
 *  OpenSCAD (www.openscad.org)
 *  Copyright (C) 2009-2011 Clifford Wolf <clifford@clifford.at> and
 *                          Marius Kintel <marius@kintel.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  As a special exception, you have permission to link this program
 *  with the CGAL library and distribute executables, as long as you
 *  follow the requirements of the GNU GPL in regard to all of the
 *  software in the executable aside from CGAL.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <sstream>
#include <cassert>
#include <cmath>
#include <vector>

#include <boost/assign/std/vector.hpp>

#include "utils/printutils.h"
#include "utils/calc.h"
#include "utils/degree_trig.h"

#include <geometry/Polygon2d.h>
#include <geometry/PolySet.h>

#include "BuiltinModule.h"
#include "AbstractNode.h"

#include "Children.h"
#include "Builtins.h"
#include "Parameters.h"
#include "Arguments.h"
#include "ModuleInstantiation.h"


#include <core/primitives/CubeNode.h>
#include <core/primitives/SphereNode.h>
#include <core/primitives/CylinderNode.h>
#include <core/primitives/PolyhedronNode.h>
#include <core/primitives/SquareNode.h>
#include <core/primitives/CircleNode.h>
#include <core/primitives/PolygonNode.h>

using namespace boost::assign; // bring 'operator+=()' into scope

#define F_MINIMUM 0.01

namespace {
   struct point2d {
     double x, y;
   };

   struct point3d {
     double x, y, z;
   };

   void generate_circle(point2d *circle, double r, int fragments)
   {
     for (int i = 0; i < fragments; ++i) {
       double phi = (360.0 * i) / fragments;
       circle[i].x = r * cos_degrees(phi);
       circle[i].y = r * sin_degrees(phi);
     }
   }

   /**
    * Return a radius value by looking up both a diameter and radius variable.
    * The diameter has higher priority, so if found an additionally set radius
    * value is ignored.
    *
    * @param parameters parameters with variable values.
    * @param inst containing instantiation.
    * @param radius_var name of the variable to lookup for the radius value.
    * @param diameter_var name of the variable to lookup for the diameter value.
    * @return radius value of type Value::Type::NUMBER or Value::Type::UNDEFINED if both
    *         variables are invalid or not set.
    */
   Value lookup_radius(const Parameters& parameters, const ModuleInstantiation *inst,
     const std::string& diameter_var, const std::string& radius_var)
   {
     const auto& d = parameters[diameter_var];
     const auto& r = parameters[radius_var];
     const auto r_defined = (r.type() == Value::Type::NUMBER);

     if (d.type() == Value::Type::NUMBER) {
       if (r_defined) {
         LOG(message_group::Warning, inst->location(), parameters.documentRoot(),
             "Ignoring radius variable '%1$s' as diameter '%2$s' is defined too.", radius_var, diameter_var);
       }
       return d.toDouble() / 2.0;
     } else if (r_defined) {
       return r.clone();
     } else {
       return Value::undefined.clone();
     }
   }

   void set_fragments(const Parameters& parameters,
     const ModuleInstantiation *inst, double& fn, double& fs, double& fa)
   {
     fn = parameters["$fn"].toDouble();
     fs = parameters["$fs"].toDouble();
     fa = parameters["$fa"].toDouble();

     if (fs < F_MINIMUM) {
       LOG(message_group::Warning, inst->location(), parameters.documentRoot(),
           "$fs too small - clamping to %1$f", F_MINIMUM);
       fs = F_MINIMUM;
     }
     if (fa < F_MINIMUM) {
       LOG(message_group::Warning, inst->location(), parameters.documentRoot(),
           "$fa too small - clamping to %1$f", F_MINIMUM);
       fa = F_MINIMUM;
     }
   }

   std::shared_ptr<AbstractNode>
   builtin_cube(const ModuleInstantiation *inst, Arguments arguments, const Children& children)
   {
     if (!children.empty()) {
       LOG(message_group::Warning, inst->location(), arguments.documentRoot(),
           "module %1$s() does not support child modules", inst->name());
     }

     Parameters parameters = Parameters::parse(std::move(arguments), inst->location(), {"size", "center"});

     constexpr double defXYZ = CubeNode::defaultXYZ;
     point3d pt = { defXYZ, defXYZ, defXYZ};
     bool center = CubeNode::defaultCenter;

     const auto& size = parameters["size"];
     if (size.isDefined()) {
       bool converted = false;
       switch( size.type()) {
       case Value::Type::NUMBER:
          converted = size.getDouble(pt.x);
          if ( converted ){
             pt.z = pt.y = pt.x;
          }
          break;
       case Value::Type::VECTOR:
          converted = size.getVec3(pt.x,pt.y,pt.z);
          break;
       default:
          converted = false;
          break;
       }
       if (!converted) {
         LOG(message_group::Warning, inst->location(), parameters.documentRoot(),
         "Unable to convert cube(size=%1$s, ...) parameter to a number or a vec3 of numbers", size.toEchoStringNoThrow());
       } else if (OpenSCAD::rangeCheck) {
         bool const ok = (pt.x > 0) && (pt.y > 0) && (pt.z > 0)
           && std::isfinite(pt.x) && std::isfinite(pt.y) && std::isfinite(pt.z);
         if (!ok) {
           LOG(message_group::Warning, inst->location(), parameters.documentRoot(),
               "cube(size=%1$s, ...)", size.toEchoStringNoThrow());
         }
       }
     }
     if (parameters["center"].type() == Value::Type::BOOL) {
         center = parameters["center"].toBool();
     }
     return std::make_shared<CubeNode>(inst,pt.x,pt.y,pt.z,center);
   }

   std::shared_ptr<AbstractNode>
   builtin_sphere(const ModuleInstantiation *inst, Arguments arguments, const Children& children)
   {
     if (!children.empty()) {
       LOG(message_group::Warning, inst->location(), arguments.documentRoot(),
           "module %1$s() does not support child modules", inst->name());
     }

     Parameters parameters = Parameters::parse(std::move(arguments), inst->location(), {"r"}, {"d"});

     double fn;
     double fs;
     double fa;
     // set from The "global special variables"
     set_fragments(parameters, inst, fn, fs, fa);
     const auto sphereRadiusValue = lookup_radius(parameters, inst, "d", "r");
     double sphereRadius = SphereNode::defaultRadius;
     if (sphereRadiusValue.type() == Value::Type::NUMBER) {
       double const r = sphereRadiusValue.toDouble();
       if (OpenSCAD::rangeCheck && (r <= 0 || !std::isfinite(r))) {
         LOG(message_group::Warning, inst->location(), parameters.documentRoot(),
             "sphere(r=%1$s)", sphereRadiusValue.toEchoStringNoThrow());
       }else{
         sphereRadius = r;
       }
     }// TODO else warn?
     return std::make_shared<SphereNode>(inst, sphereRadius,fn,fs,fa);
   }

   std::shared_ptr<AbstractNode>
   builtin_cylinder(const ModuleInstantiation *inst, Arguments arguments, const Children& children)
   {

     if (!children.empty()) {
       LOG(message_group::Warning, inst->location(), arguments.documentRoot(),
           "module %1$s() does not support child modules", inst->name());
     }

     Parameters parameters = Parameters::parse(std::move(arguments), inst->location(),
       {"h", "r1", "r2", "center"}, {"r", "d", "d1", "d2"});

     struct cyl{
         double r1 = 1;
         double r2 = 1;
         double h = 1;
         double fn;
         double fs;
         double fa;
         bool center = false;
     } cyl;

     set_fragments(parameters, inst, cyl.fn, cyl.fs, cyl.fa);
     if (parameters["h"].type() == Value::Type::NUMBER) {
       cyl.h = parameters["h"].toDouble();
     }

     auto r = lookup_radius(parameters, inst, "d", "r");
     auto r1 = lookup_radius(parameters, inst, "d1", "r1");
     auto r2 = lookup_radius(parameters, inst, "d2", "r2");
     if (r.type() == Value::Type::NUMBER &&
         (r1.type() == Value::Type::NUMBER || r2.type() == Value::Type::NUMBER)
         ) {
       LOG(message_group::Warning, inst->location(), parameters.documentRoot(), "Cylinder parameters ambiguous");
     }

     if (r.type() == Value::Type::NUMBER) {
       cyl.r1 = r.toDouble();
       cyl.r2 = r.toDouble();
     }
     if (r1.type() == Value::Type::NUMBER) {
       cyl.r1 = r1.toDouble();
     }
     if (r2.type() == Value::Type::NUMBER) {
       cyl.r2 = r2.toDouble();
     }

     if (OpenSCAD::rangeCheck) {
       if (cyl.h <= 0 || !std::isfinite(cyl.h)) {
         LOG(message_group::Warning, inst->location(), parameters.documentRoot(),
            "cylinder(h=%1$s, ...)", parameters["h"].toEchoStringNoThrow());
       }
       if (cyl.r1 < 0 || cyl.r2 < 0 ||
             (cyl.r1 == 0 && cyl.r2 == 0) ||
                !std::isfinite(cyl.r1) || !std::isfinite(cyl.r2)) {
         LOG(message_group::Warning, inst->location(), parameters.documentRoot(),
             "cylinder(r1=%1$s, r2=%2$s, ...)",
             (r1.type() == Value::Type::NUMBER ? r1.toEchoStringNoThrow() : r.toEchoStringNoThrow()),
             (r2.type() == Value::Type::NUMBER ? r2.toEchoStringNoThrow() : r.toEchoStringNoThrow()));
       }
     }

     if (parameters["center"].type() == Value::Type::BOOL) {
       cyl.center = parameters["center"].toBool();
     }
     return  std::make_shared<CylinderNode>(inst,cyl.r1,cyl.r2,cyl.h,cyl.fn,cyl.fs,cyl.fa,cyl.center);
   }

   std::shared_ptr<AbstractNode>
   builtin_polyhedron(const ModuleInstantiation *inst, Arguments arguments, const Children& children)
   {

     if (!children.empty()) {
       LOG(message_group::Warning, inst->location(), arguments.documentRoot(),
           "module %1$s() does not support child modules", inst->name());
     }

     Parameters parameters = Parameters::parse(std::move(arguments), inst->location(),
       {"points", "faces", "convexity"}, {"triangles"});

     if (parameters["points"].type() != Value::Type::VECTOR) {
       LOG(message_group::Error, inst->location(), parameters.documentRoot(),
        "Unable to convert points = %1$s to a vector of coordinates",
           parameters["points"].toEchoStringNoThrow());
       return std::make_shared<PolyhedronNode>(inst);
     }

     std::vector<primitives::point3d> points;
     for (const Value& pointValue : parameters["points"].toVector()) {
       primitives::point3d point;
       if (!pointValue.getVec3(point.x, point.y, point.z, 0.0) ||
           !std::isfinite(point.x) || !std::isfinite(point.y) || !std::isfinite(point.z)
           ) {
         LOG(message_group::Error, inst->location(), parameters.documentRoot(),
           "Unable to convert points[%1$d] = %2$s to a vec3 of numbers",
             points.size(), pointValue.toEchoStringNoThrow());
         points.push_back({0, 0, 0});
       } else {
         points.push_back(std::move(point));
       }
     }

     const Value *pFaceVect = nullptr;
     if ( parameters["faces"].type() == Value::Type::UNDEFINED &&
           parameters["triangles"].type() != Value::Type::UNDEFINED) {
       // backwards compatible
       LOG(message_group::Deprecated, inst->location(), parameters.documentRoot(),
         "polyhedron(triangles=[]) will be removed in future releases. Use polyhedron(faces=[]) instead.");
       pFaceVect = &parameters["triangles"];
     } else {
       pFaceVect = &parameters["faces"];
     }
     if (pFaceVect->type() != Value::Type::VECTOR) {
       LOG(message_group::Error, inst->location(), parameters.documentRoot(),
         "Unable to convert faces = %1$s to a vector of vector of point indices", pFaceVect->toEchoStringNoThrow());
       return std::make_shared<PolyhedronNode>(inst);
     }

     std::vector<std::vector<size_t>> faces;
     size_t faceIndex = 0;
     auto const & faceValues = pFaceVect->toVector();
     for (const Value& faceValue :faceValues) {
       if (faceValue.type() != Value::Type::VECTOR) {
         LOG(message_group::Error, inst->location(), parameters.documentRoot(),
            "Unable to convert faces[%1$d] = %2$s to a vector of numbers",
                faceIndex, faceValue.toEchoStringNoThrow());
       } else {
         size_t pointIndexIdx = 0;
         std::vector<size_t> face;
         auto const & faceValueVect = faceValue.toVector();
         for (const Value& pointIndexValue : faceValueVect) {
           if (pointIndexValue.type() != Value::Type::NUMBER) {
             LOG(message_group::Error, inst->location(), parameters.documentRoot(),
                "Unable to convert faces[%1$d][%2$d] = %3$s to a number",
                    faceIndex, pointIndexIdx, pointIndexValue.toEchoStringNoThrow());
           } else {
             auto const pointIndex = (size_t)pointIndexValue.toDouble();
             if (pointIndex < points.size()) {
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
           faces.push_back(std::move(face));
         }
       }
       faceIndex++;
     }

     int const convexity = std::max(static_cast<int>(parameters["convexity"].toDouble()),1);

     return std::make_shared<PolyhedronNode>(
        inst, std::move(points),std::move(faces),convexity
     );
   }

   std::shared_ptr<AbstractNode>
   builtin_square(const ModuleInstantiation *inst, Arguments arguments, const Children& children)
   {

     if (!children.empty()) {
       LOG(message_group::Warning, inst->location(), arguments.documentRoot(),
           "module %1$s() does not support child modules", inst->name());
     }

     Parameters parameters = Parameters::parse(std::move(arguments), inst->location(), {"size", "center"});

     struct square{
       double x = 1;
       double y = 1;
       bool center = false;
     } square;

     const auto& size = parameters["size"];

     if (size.isDefined()) {
       bool converted = false;
       converted |= size.getDouble(square.x);
       converted |= size.getDouble(square.y);
       converted |= size.getVec2(square.x, square.y);
       if (!converted) {
         LOG(message_group::Warning, inst->location(), parameters.documentRoot(),
            "Unable to convert square(size=%1$s, ...) parameter to a number or a vec2 of numbers",
               size.toEchoStringNoThrow());
       } else if (OpenSCAD::rangeCheck) {
         bool ok = true;
         ok &= (square.x > 0) && (square.y > 0);
         ok &= std::isfinite(square.x) && std::isfinite(square.y);
         if (!ok) {
           LOG(message_group::Warning, inst->location(), parameters.documentRoot(),
             "square(size=%1$s, ...)", size.toEchoStringNoThrow());
         }
       }
     }
     if (parameters["center"].type() == Value::Type::BOOL) {
       square.center = parameters["center"].toBool();
     }
     return std::make_shared<SquareNode>(inst,square.x,square.y,square.center);
   }

   std::shared_ptr<AbstractNode>
   builtin_circle(const ModuleInstantiation *inst, Arguments arguments, const Children& children)
   {
      if (!children.empty()) {
         LOG(message_group::Warning, inst->location(), arguments.documentRoot(),
         "module %1$s() does not support child modules", inst->name());
      }

      Parameters parameters = Parameters::parse(std::move(arguments), inst->location(), {"r"}, {"d"});

      struct circle{
         double fn;
         double fa;
         double fs;
         double r =1;
      }circle;
      set_fragments(parameters, inst, circle.fn, circle.fs, circle.fa);
      const auto r = lookup_radius(parameters, inst, "d", "r");
      if (r.type() == Value::Type::NUMBER) {
         circle.r = r.toDouble();
         if (OpenSCAD::rangeCheck && ((circle.r <= 0) || !std::isfinite(circle.r))) {
            LOG(message_group::Warning, inst->location(), parameters.documentRoot(),
            "circle(r=%1$s)", r.toEchoStringNoThrow());
         }
      }

      return std::make_shared<CircleNode>(
         inst,circle.fn,circle.fa,circle.fs,circle.r
      );
   }

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
     std::vector<primitives::point2d> points;
     auto const & pointsVect = parameters["points"].toVector();
     for (const Value& pointValue : pointsVect) {
       primitives::point2d point;
       if (!pointValue.getVec2(point.x, point.y) ||
           !std::isfinite(point.x) || !std::isfinite(point.y)
           ) {
         LOG(message_group::Error, inst->location(), parameters.documentRoot(),
           "Unable to convert points[%1$d] = %2$s to a vec2 of numbers",
               points.size(), pointValue.toEchoStringNoThrow());
         points.push_back({0, 0});
       } else {
         points.push_back(point);
       }
     }

     std::vector<std::vector<size_t> > paths;
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
               if (pointIndex < points.size()) {
                 path.push_back(pointIndex);
               } else {
                 LOG(message_group::Warning, inst->location(), parameters.documentRoot(),
                   "Point index %1$d is out of bounds (from paths[%2$d][%3$d])",
                       pointIndex, pathIndex, pointIndexIdx);
               }
             }
             pointIndexIdx++;
           }
           paths.push_back(std::move(path));
         }
         pathIndex++;
       }
     } else if (parameters["paths"].type() != Value::Type::UNDEFINED) {
       LOG(message_group::Error, inst->location(), parameters.documentRoot(),
          "Unable to convert paths = %1$s to a vector of vector of point indices", parameters["paths"].toEchoStringNoThrow());
       return std::make_shared<PolygonNode>(inst);
     }
     int const convexity = std::max(static_cast<int>(parameters["convexity"].toDouble()),1);

     return  std::make_shared<PolygonNode>(inst,std::move(points),std::move(paths),convexity);
   }
}//~namespace

void register_builtin_primitives()
{
  Builtins::init("cube", new BuiltinModule(builtin_cube),
  {
    "cube(size)",
    "cube([width, depth, height])",
    "cube([width, depth, height], center = true)",
  });

  Builtins::init("sphere", new BuiltinModule(builtin_sphere),
  {
    "sphere(radius)",
    "sphere(r = radius)",
    "sphere(d = diameter)",
  });

  Builtins::init("cylinder", new BuiltinModule(builtin_cylinder),
  {
    "cylinder(h, r1, r2)",
    "cylinder(h = height, r = radius, center = true)",
    "cylinder(h = height, r1 = bottom, r2 = top, center = true)",
    "cylinder(h = height, d = diameter, center = true)",
    "cylinder(h = height, d1 = bottom, d2 = top, center = true)",
  });

  Builtins::init("polyhedron", new BuiltinModule(builtin_polyhedron),
  {
    "polyhedron(points, faces, convexity)",
  });

  Builtins::init("square", new BuiltinModule(builtin_square),
  {
    "square(size, center = true)",
    "square([width,height], center = true)",
  });

  Builtins::init("circle", new BuiltinModule(builtin_circle),
  {
    "circle(radius)",
    "circle(r = radius)",
    "circle(d = diameter)",
  });

  Builtins::init("polygon", new BuiltinModule(builtin_polygon),
  {
    "polygon([points])",
    "polygon([points], [paths])",
  });
}
