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

#include "BuiltinModule.h"

#include "Children.h"
#include "Builtins.h"
#include "Parameters.h"
#include "Value.h"
#include "ModuleInstantiation.h"

#include <core/primitives/primitives.h>
#include <core/primitives/CubeNode.h>
#include <core/primitives/SphereNode.h>
#include <core/primitives/CylinderNode.h>
#include <core/primitives/PolyhedronNode.h>
#include <core/primitives/SquareNode.h>
#include <core/primitives/CircleNode.h>
#include <core/primitives/PolygonNode.h>

[[nodiscard]] bool isNumber(Value const & v)
{
  return v.type() == Value::Type::NUMBER;
}

[[nodiscard]] bool isVector(Value const & v)
{
  return v.type() == Value::Type::VECTOR;
}
[[nodiscard]] bool isBool(Value const & v)
{
  return v.type() == Value::Type::BOOL;
}

[[nodiscard]] bool isDefined(Value const & v)
{
  return v.type() != Value::Type::UNDEFINED;
}
[[nodiscard]] bool isUndefined(Value const & v)
{
  return v.type() == Value::Type::UNDEFINED;
}

namespace primitives{

    bool get_center(Parameters const & parameters,std::variant<bool, point3di>& centerOut)
    {
     const auto& centerValue = parameters["center"];
     if ( centerValue.isDefined()){
        bool converted = false;
        switch(centerValue.type()){
          case Value::Type::BOOL:
            centerOut = centerValue.toBool();
            converted = true;
            break;
          case Value::Type::VECTOR:{
           point3d c1;
           converted = centerValue.getVec3(c1.x,c1.y,c1.z);
           if ( converted){
              auto sign = [] (double v) {
                 int const vi = static_cast<int>(v);
                 return (vi < 0)
                   ? -1
                   : ((vi > 0)?1:0)
                ;
              };
              centerOut = point3di{sign(c1.x),sign(c1.y),sign(c1.z)};
           }
           break;
        }
        default:
            LOG(message_group::Warning, parameters.location(),parameters.documentRoot(),"cube center unknown type");
           break;
        }
        if (!converted){
            LOG(message_group::Warning,parameters.location(),parameters.documentRoot(),
           "Unable to convert cube(.../center=%1$s, ...) parameter to a bool or a vec3 of signs", centerValue.toEchoStringNoThrow());
        }
        return converted;
     }
     return false;
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

     if (fs < primitives::facet_params_t::fsMin) {
       fs = primitives::facet_params_t::fsMin;
       LOG(message_group::Warning, inst->location(), parameters.documentRoot(),
           "$fs too small - clamping to %1$f",fs);

     }
     if (fa < primitives::facet_params_t::faMin) {
       fa = primitives::facet_params_t::faMin;
       LOG(message_group::Warning, inst->location(), parameters.documentRoot(),
           "$fa too small - clamping to %1$f", fa);
     }
   }

   std::shared_ptr<AbstractNode>
   builtin_cube(const ModuleInstantiation *inst, Arguments arguments, const Children& children);

   std::shared_ptr<AbstractNode>
   builtin_sphere(const ModuleInstantiation *inst, Arguments arguments, const Children& children);

   std::shared_ptr<AbstractNode>
   builtin_cylinder(const ModuleInstantiation *inst, Arguments arguments, const Children& children);

   std::shared_ptr<AbstractNode>
   builtin_polyhedron(const ModuleInstantiation *inst, Arguments arguments, const Children& children);

   std::shared_ptr<AbstractNode>
   builtin_square(const ModuleInstantiation *inst, Arguments arguments, const Children& children);

   std::shared_ptr<AbstractNode>
   builtin_circle(const ModuleInstantiation *inst, Arguments arguments, const Children& children);

   std::shared_ptr<AbstractNode>
   builtin_polygon(const ModuleInstantiation *inst, Arguments arguments, const Children& children);
}//~namespace primitives

void register_builtin_primitives()
{
  Builtins::init("cube", new BuiltinModule(primitives::builtin_cube),
  {
    "cube(size)",
    "cube([width, depth, height])",
    "cube([width, depth, height], center = true)",
  });

  Builtins::init("sphere", new BuiltinModule(primitives::builtin_sphere),
  {
    "sphere(radius)",
    "sphere(r = radius)",
    "sphere(d = diameter)",
  });

  Builtins::init("cylinder", new BuiltinModule(primitives::builtin_cylinder),
  {
    "cylinder(h, r1, r2)",
    "cylinder(h = height, r = radius, center = true)",
    "cylinder(h = height, r1 = bottom, r2 = top, center = true)",
    "cylinder(h = height, d = diameter, center = true)",
    "cylinder(h = height, d1 = bottom, d2 = top, center = true)",
  });

  Builtins::init("polyhedron", new BuiltinModule(primitives::builtin_polyhedron),
  {
    "polyhedron(points, faces, convexity)",
  });

  Builtins::init("square", new BuiltinModule(primitives::builtin_square),
  {
    "square(size, center = true)",
    "square([width,height], center = true)",
  });

  Builtins::init("circle", new BuiltinModule(primitives::builtin_circle),
  {
    "circle(radius)",
    "circle(r = radius)",
    "circle(d = diameter)",
  });

  Builtins::init("polygon", new BuiltinModule(primitives::builtin_polygon),
  {
    "polygon([points])",
    "polygon([points], [paths])",
  });
}
