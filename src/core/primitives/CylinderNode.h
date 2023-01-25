#pragma once

#include <string>

#include <core/AbstractNode.h>

class Geometry;

class CylinderNode : public LeafNode{
public:
  CylinderNode(const ModuleInstantiation *mi,
    primitives::cylinder_params_t && params
  ) : LeafNode(mi),params{params}{}
  std::string toString() const override;
  std::string name() const override { return "cylinder"; }
  const Geometry *createGeometry() const override;

  primitives::cylinder_params_t const params;
};
