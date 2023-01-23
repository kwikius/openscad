#pragma once

#include "ModuleInstantiation.h"

/*!
   Only instantiated once, for the top-level file.
 */
class RootNode final : public Visitable<GroupNode, RootNode>{
public:
  RootNode( std::shared_ptr<const ModuleInstantiation> && rootIn)
  : Visitable{rootIn.get()},root_mi{rootIn}{ }
  std::string name() const override;
private:
  std::shared_ptr<const ModuleInstantiation> root_mi;
};


