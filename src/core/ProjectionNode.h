#pragma once

#include "AbstractNode.h"
#include <string>

class ProjectionNode final : public Visitable<AbstractPolyNode, ProjectionNode>{
public:
  ProjectionNode(NodeParams const & np)
  : Visitable{np}, convexity(1), cut_mode(false) { }
  std::string toString() const override;
  std::string name() const override { return "projection"; }

  int convexity;
  bool cut_mode;
};
