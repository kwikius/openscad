#pragma once

#include <string>
#include <sstream>

#include <core/AbstractNode.h>
#include "primitives.h"
class Geometry;

class SphereNode : public LeafNode{
public:
  SphereNode(const ModuleInstantiation *mi,
    primitives::sphere_params_t && params
  ) : LeafNode(mi),params{params} {}
  std::string toString() const override;
  std::string name() const override { return "sphere"; }
  const Geometry *createGeometry() const override;
  primitives::sphere_params_t const params;
};
