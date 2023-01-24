#pragma once

#include <string>

#include <core/AbstractNode.h>

class Geometry;

class CylinderNode : public LeafNode{
public:
  CylinderNode(const ModuleInstantiation *mi,
         double r1,
         double r2,
         double  h,
         double fn,
         double fs,
         double fa,
         bool center
  ) : LeafNode(mi),r1{r1},r2{r2},h{h},fn{fn},fs{fs},fa{fa},center{center}{}
  std::string toString() const override;
  std::string name() const override { return "cylinder"; }
  const Geometry *createGeometry() const override;

  double const r1;
  double const r2;
  double const h;
  double const fn;
  double const fs;
  double const fa;
  bool const center;
};
