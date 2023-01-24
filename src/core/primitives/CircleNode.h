#pragma once


#include <string>

#include <core/AbstractNode.h>

class CircleNode : public LeafNode{
public:
  CircleNode(const ModuleInstantiation *mi,
    double fn, double fa, double fs, double r
  ) : LeafNode(mi),fn{fn},fa{fa},fs{fs},r{r} {}
  std::string toString() const override;
  std::string name() const override { return "circle"; }
  const Geometry* createGeometry() const override;

  double const fn;
  double const fa;
  double const fs;
  double const r;
};
