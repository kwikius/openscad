#pragma once

#include <string>

#include <core/AbstractNode.h>

class Geometry;

class SquareNode : public LeafNode{
public:
  SquareNode(const ModuleInstantiation *mi, double x, double y, bool center)
  : LeafNode(mi),x{x},y{y},center{center} {}
  std::string toString() const override;
  std::string name() const override { return "square"; }
  const Geometry *createGeometry() const override;

  double const x;
  double const y;
  bool const center;
};

