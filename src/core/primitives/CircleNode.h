#pragma once


#include <string>

#include <core/AbstractNode.h>
#include "primitives.h"

class Geometry;

namespace primitives{

   struct circle_params_t{
      double r = 1 ;
      facet_params_t fp;
   };
}

class CircleNode : public LeafNode{
public:
  CircleNode(const ModuleInstantiation *mi,
    primitives::circle_params_t && params
  ) : LeafNode(mi), params{params}{}
  std::string toString() const override;
  std::string name() const override { return "circle"; }
  const Geometry* createGeometry() const override;
  primitives::circle_params_t const params;
};
