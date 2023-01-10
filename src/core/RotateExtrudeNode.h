#pragma once

#include "AbstractNode.h"
#include "Filename.h"

class RotateExtrudeNode : public Visitable<AbstractPolyNode,RotateExtrudeNode>{
public:
  RotateExtrudeNode(const ModuleInstantiation *mi) : Visitable(mi) {
    convexity = 0;
    fn = fs = fa = 0;
    origin_x = origin_y = scale = 0;
    angle = 360;
  }
  std::string toString() const override;
  std::string name() const override { return "rotate_extrude"; }

  int convexity;
  double fn, fs, fa;
  double origin_x, origin_y, scale, angle;
  Filename filename;
  std::string layername;
};
