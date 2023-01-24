#pragma once

#include <string>
#include <sstream>

#include <core/AbstractNode.h>

class Geometry;

class CubeNode : public LeafNode{
public:
  CubeNode(const ModuleInstantiation *mi,
    double x, double y, double z, bool center)
  : LeafNode(mi)
  ,x{x}
  ,y{y}
  ,z{z}
  ,center{center}{}

  std::string toString() const override;
  std::string name() const override { return "cube"; }
  const Geometry *createGeometry() const override;

  static constexpr double defaultXYZ = 1.0;
  static constexpr bool defaultCenter = false;

  double const x;
  double const y;
  double const z;
  bool const center;
};
