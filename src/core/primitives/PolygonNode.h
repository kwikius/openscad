#pragma once

#include <string>
#include <vector>

#include <core/AbstractNode.h>
#include "primitives.h"

class Geometry;

namespace primitives{

   struct polygon_params_t{
      std::vector<primitives::point2d> points;
      std::vector<std::vector<size_t>> paths;
      int convexity = 1;
   };
}

class PolygonNode : public LeafNode{
public:
   PolygonNode (NodeParams const & np,
     primitives::polygon_params_t && params
   ): LeafNode{np}, params{params} {}
   std::string toString() const override;
   std::string name() const override { return "polygon"; }
   const Geometry *createGeometry() const override;
   primitives::polygon_params_t const params;
};

