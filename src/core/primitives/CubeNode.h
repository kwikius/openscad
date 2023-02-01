#pragma once

#include <string>
#include <sstream>
#include <variant>

#include <core/AbstractNode.h>
#include "primitives.h"

class Geometry;

namespace primitives{

   struct cube_params_t {
      point3d size = point3d{1,1,1};
      std::variant<bool, point3di> center = false;
   };
}

class CubeNode : public LeafNode{
public:
  CubeNode(NodeParams const & np,
    primitives::cube_params_t && params)
  : LeafNode{np},params{params}{}

  std::string toString() const override;
  std::string name() const override { return "cube"; }
  const Geometry *createGeometry() const override;
  primitives::cube_params_t const params;
};
