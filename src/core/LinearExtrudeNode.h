#pragma once

#include "AbstractNode.h"
#include "Filename.h"

class LinearExtrudeNode : public Visitable<AbstractPolyNode, LinearExtrudeNode>{
public:
  LinearExtrudeNode(const ModuleInstantiation *mi) : Visitable(mi) {}
  std::string toString() const override;
  std::string name() const override { return "linear_extrude"; }

  double height = 100.0;
  double origin_x = 0.0, origin_y = 0.0;
  double fn = 0.0, fs = 0.0, fa = 0.0;
  double scale_x = 1.0, scale_y = 1.0;
  double twist = 0.0;
  unsigned int convexity = 1u;
  unsigned int slices = 1u, segments = 0u;
  bool has_twist = false, has_slices = false, has_segments = false;
  bool center = false;

  Filename filename;
  std::string layername;
};
