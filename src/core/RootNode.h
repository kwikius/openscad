#pragma once

#include "ModuleInstantiation.h"

/*!
   Only instantiated once, for the top-level file.
 */
class RootNode final : public Visitable<GroupNode, RootNode>{
public:
  RootNode() : Visitable(&mi), mi("group") { }
  std::string name() const override;
private:
  ModuleInstantiation mi;
};


