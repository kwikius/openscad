#pragma once

#include <string>
#include <vector>

#include <core/AbstractNode.h>
#include "primitives.h"

class PolygonNode : public LeafNode
{
public:
   PolygonNode (const ModuleInstantiation *mi)
   : LeafNode(mi),convexity{1}{}

   PolygonNode (const ModuleInstantiation *mi,
      std::vector<primitives::point2d> && points,
      std::vector<std::vector<size_t>> && paths,
      int convexity
   ): LeafNode(mi), points{points},paths{paths},convexity{convexity} {}
   std::string toString() const override;
   std::string name() const override { return "polygon"; }
   const Geometry *createGeometry() const override;

   std::vector<primitives::point2d> const points;
   std::vector<std::vector<size_t>> const  paths;
   int const convexity;
};

