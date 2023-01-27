#pragma once

#include <string>

#include <core/AbstractNode.h>
#include "primitives.h"

class Geometry;

namespace primitives{

   struct square_params_t{
      point2d size = point2d{1,1};
      bool center = false;
   };
}

class SquareNode : public LeafNode{
public:
  SquareNode(NodeParams const & np,
  primitives::square_params_t && params)
  : LeafNode{np},params{params} {}
  std::string toString() const override;
  std::string name() const override { return "square"; }
  const Geometry *createGeometry() const override;
  primitives::square_params_t const params;
};

