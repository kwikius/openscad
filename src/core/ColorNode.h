#pragma once

#include <geometry/linalg.h>

#include "node.h"

class ColorNode : public AbstractNode
{
public:
  VISITABLE();
  ColorNode(const ModuleInstantiation *mi) : AbstractNode(mi), color(-1.0f, -1.0f, -1.0f, 1.0f) { }
  std::string toString() const override;
  std::string name() const override;

  Color4f color;
};
