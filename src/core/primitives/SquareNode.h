#pragma once

#include <string>

#include <core/AbstractNode.h>
#include "primitives.h"

class Geometry;

class SquareNode : public LeafNode{
public:
  SquareNode(const ModuleInstantiation *mi,
  primitives::square_params_t && params)
  : LeafNode(mi),params{params} {}
  std::string toString() const override;
  std::string name() const override { return "square"; }
  const Geometry *createGeometry() const override;
  primitives::square_params_t const params;
};

