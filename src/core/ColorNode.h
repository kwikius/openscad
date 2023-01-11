#pragma once

#include <geometry/linalg.h>

#include "AbstractNode.h"

class ColorNode final : public Visitable<AbstractNode, ColorNode>{
public:
  ColorNode(const ModuleInstantiation *mi)
 : Visitable(mi), color(-1.0f, -1.0f, -1.0f, 1.0f) { }
  std::string toString() const override;
  std::string name() const override;

  Color4f color;
};
