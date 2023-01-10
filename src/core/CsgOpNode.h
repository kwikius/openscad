#pragma once

#include <string>

#include "AbstractNode.h"
#include "enums.h"

class CsgOpNode : public Visitable<AbstractNode, CsgOpNode>{
public:

  OpenSCADOperator type;
  CsgOpNode(const ModuleInstantiation *mi, OpenSCADOperator type)
  : Visitable(mi), type(type) { }
  std::string toString() const override;
  std::string name() const override;
};
