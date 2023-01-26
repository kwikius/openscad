#pragma once

#include <string>
#include <vector>

#include <core/AbstractNode.h>
#include "primitives.h"

namespace primitives{

   struct polyhedron_params_t {
      std::vector<primitives::point3d> points;
      std::vector<std::vector<size_t>> faces;
      int convexity = 1;
   };
}

class PolyhedronNode : public LeafNode{
public:
  PolyhedronNode (const ModuleInstantiation *mi)
  : LeafNode(mi){}
  PolyhedronNode (const ModuleInstantiation *mi,
     primitives::polyhedron_params_t && params
  ) : LeafNode{mi}, params{params}{}
  std::string toString() const override;
  std::string name() const override { return "polyhedron"; }
  const Geometry *createGeometry() const override;
  primitives::polyhedron_params_t const params;
};
