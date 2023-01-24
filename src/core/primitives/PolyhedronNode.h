#pragma once

#include <string>
#include <vector>

#include <core/AbstractNode.h>
#include "primitives.h"

class PolyhedronNode : public LeafNode{
public:
  PolyhedronNode (const ModuleInstantiation *mi) : LeafNode(mi), convexity{1} {}
  PolyhedronNode (const ModuleInstantiation *mi,
     std::vector<primitives::point3d> && points,
     std::vector<std::vector<size_t>> && faces,
     int convexity
  ) : LeafNode{mi}, points{points},faces{faces},convexity{convexity}{}
  std::string toString() const override;
  std::string name() const override { return "polyhedron"; }
  const Geometry *createGeometry() const override;

  std::vector<primitives::point3d> const points;
  std::vector<std::vector<size_t>> const  faces;
  int const convexity;
};
