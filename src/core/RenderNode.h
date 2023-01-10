#pragma once

#include "AbstractNode.h"
#include <string>

class RenderNode : public Visitable<AbstractNode, RenderNode>{
public:
  RenderNode(const ModuleInstantiation *mi) : Visitable(mi) { }
  std::string toString() const override;
  std::string name() const override { return "render"; }

  int convexity{1};
};
