#pragma once

#include "AbstractNode.h"
#include "ext/polyclipping/clipper.hpp"

class OffsetNode final : public Visitable<AbstractPolyNode, OffsetNode>{
public:
  OffsetNode(NodeParams const & np) : Visitable{np} { }
  std::string toString() const override;
  std::string name() const override { return "offset"; }

  bool chamfer{false};
  double fn{0}, fs{0}, fa{0}, delta{1};
  double miter_limit{1000000.0}; // currently fixed high value to disable chamfers with jtMiter
  ClipperLib::JoinType join_type{ClipperLib::jtRound};
};
