#pragma once

#include "AbstractNode.h"
#include <string>

class RenderNode final : public Visitable<AbstractNode, RenderNode>{
public:
  RenderNode(NodeParams const & np) : Visitable{np} { }
  std::string toString() const override;
  std::string name() const override { return "render"; }

  int convexity{1};
};
