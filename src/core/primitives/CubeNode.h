#pragma once

#include <string>
#include <sstream>

#include <core/AbstractNode.h>

class Geometry;

class CubeNode : public LeafNode{
public:
  CubeNode(const ModuleInstantiation *mi,
    primitives::cube_params_t && params)
  : LeafNode(mi),params{params}{}

  std::string toString() const override;
  std::string name() const override { return "cube"; }
  const Geometry *createGeometry() const override;
  primitives::cube_params_t const params;
};
