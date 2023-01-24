#pragma once

#include <string>
#include <sstream>

#include <core/AbstractNode.h>

class Geometry;

class SphereNode : public LeafNode{
public:
  SphereNode(const ModuleInstantiation *mi,
     double const & radius,
     double const & fn,
     double const & fs,
     double const & fa
  ) : LeafNode(mi),r{radius},fn{fn},fs{fs},fa{fa} {}
  std::string toString() const override;
  std::string name() const override { return "sphere"; }
  const Geometry *createGeometry() const override;

  static constexpr double defaultRadius = 1.0;

  const double fn;
  const double fs;
  const double fa;
  const double r;
};
