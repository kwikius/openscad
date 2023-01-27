#pragma once

#include <string>
#include <sstream>

#include <core/AbstractNode.h>
#include "primitives.h"

class Geometry;

namespace primitives{

   struct sphere_params_t{
      double r = 1 ;
      facet_params_t fp;
   };
}

class SphereNode : public LeafNode{
public:
  SphereNode(NodeParams const & np,
    primitives::sphere_params_t && params
  ) : LeafNode(np),params{params} {}
  std::string toString() const override;
  std::string name() const override { return "sphere"; }
  const Geometry *createGeometry() const override;
  primitives::sphere_params_t const params;
};
