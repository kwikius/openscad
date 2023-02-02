
#include <cmath>

#include <core/Children.h>
#include <core/Value.h>
#include <core/Parameters.h>
#include <core/Arguments.h>
#include <core/ModuleInstantiation.h>
#include <core/Visitable_inline.h>

#include <geometry/Geometry.h>
#include <geometry/PolySet.h>

#include "CubeNode.h"

std::string CubeNode::toString() const
{
   std::ostringstream stream;
   stream << "cube(size = ["
   << this->params.size.x << ", "
   << this->params.size.y << ", "
   << this->params.size.z << "], center = ";
   if (std::holds_alternative<bool>(this->params.center)){
      bool const center = std::get<bool>(this->params.center);
      stream << (center ? "true" : "false") << ")";
   }else{
      using primitives::point3di;
      point3di center = std::get<point3di>(this->params.center);
      stream << "[" << center.x <<", " << center.y << ", " << center.z << "])";
   }
   return stream.str();
}

const Geometry *CubeNode::createGeometry() const
{
   auto p = new PolySet(3, true);

   if (!OpenSCAD::rangeCheck && !isPositiveFinite(this->params.size) ){
     return p;
   }

   using primitives::point3d;
   using primitives::point3di;
   point3di center;
   if ( std::holds_alternative<bool>(this->params.center)){
     int const c = std::get<bool>(this->params.center)?0:1;
     center = point3di{c,c,c};
   }else{
     center = std::get<point3di>(this->params.center);
   }
   point3d const size = this->params.size;
   point3d const tx = point3d{ size.x * center.x, size.y * center.y, size.z * center.z};
   point3d const p1 = (tx - size) / 2;
   point3d const p2 = (tx + size) / 2;

   double const x1 = p1.x;
   double const y1 = p1.y;
   double const z1 = p1.z;

   double const x2 = p2.x;
   double const y2 = p2.y;
   double const z2 = p2.z;

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
namespace primitives{
   std::shared_ptr<AbstractNode>
   builtin_cube(const ModuleInstantiation *inst, Arguments arguments, const Children& children)
   {
     if (!children.empty()) {
       LOG(message_group::Warning, inst->location(), arguments.documentRoot(),
           "module %1$s() does not support child modules", inst->name());
     }

     Parameters const parameters = Parameters::parse(std::move(arguments), inst->location(), {"size", "center"});

     cube_params_t cube;

     const auto& size = parameters["size"];
     if (size.isDefined()) {
       bool converted = false;
       switch( size.type()) {
       case Value::Type::NUMBER:
          converted = size.getDouble(cube.size.x);
          if ( converted ){
             cube.size.z = cube.size.y = cube.size.x;
          }
          break;
       case Value::Type::VECTOR:
          converted = size.getVec3(cube.size.x,cube.size.y,cube.size.z);
          break;
       default:
          converted = false;
          break;
       }
       if (!converted) {
         LOG(message_group::Warning, inst->location(), parameters.documentRoot(),
         "Unable to convert cube(size=%1$s, ...) parameter to a number or a vec3 of numbers", size.toEchoStringNoThrow());
          // TODO absolute invariant
       } else if (OpenSCAD::rangeCheck && ! isPositiveFinite(cube.size)) {
           LOG(message_group::Warning, inst->location(), parameters.documentRoot(),
               "cube(size=%1$s, ...)", size.toEchoStringNoThrow());
       }
     }
     primitives::get_center(parameters,cube.center);
     return std::make_shared<CubeNode>(*inst,std::move(cube));
   }
}//~namespace
