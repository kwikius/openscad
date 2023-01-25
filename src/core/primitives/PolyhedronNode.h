#pragma once

#include <string>
#include <vector>

#include <core/AbstractNode.h>
#include "primitives.h"

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
