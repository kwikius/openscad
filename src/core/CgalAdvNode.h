#pragma once

#include <geometry/linalg.h>
#include "AbstractNode.h"

enum class CgalAdvType {
  MINKOWSKI,
  HULL,
  FILL,
  RESIZE
};

class CgalAdvNode : public Visitable<AbstractNode, CgalAdvNode>{
public:
  CgalAdvNode(NodeParams const & np, CgalAdvType type)
  : Visitable{np}, type(type) {}
  std::string toString() const override;
  std::string name() const override;

  unsigned int convexity{1};
  Vector3d newsize;
  Eigen::Matrix<bool, 3, 1> autosize;
  CgalAdvType type;
};
