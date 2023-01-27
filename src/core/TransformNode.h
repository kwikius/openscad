#pragma once

#include <geometry/linalg.h>

#include "AbstractNode.h"

class TransformNode final : public Visitable<AbstractNode, TransformNode>{
public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  TransformNode(NodeParams const & np, std::string const & verbose_name);
  std::string toString() const override;
  std::string name() const override;
  std::string verbose_name() const override;
  Transform3d matrix;
private:
  const std::string _name;
};
