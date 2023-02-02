#pragma once

#include <string>

#include <core/AbstractNode.h>

class Geometry;

namespace primitives{

   struct cylinder_params_t{
      double r1 = 1;
      double r2 = 1;
      double  h = 1;
      facet_params_t fp;
      std::variant<bool, point3di> center = false;
   };
}

class CylinderNode : public LeafNode{
public:
  CylinderNode(NodeParams const & np,
    primitives::cylinder_params_t && params
  ) : LeafNode{np},params{params}{}
  std::string toString() const override;
  std::string name() const override { return "cylinder"; }
  const Geometry *createGeometry() const override;

  primitives::cylinder_params_t const params;
};
