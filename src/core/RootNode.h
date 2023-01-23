#pragma once
#include <string>

#include "AbstractNode.h"
/*!
   Only instantiated once, for the top-level file.
 */
class RootNode final : public Visitable<GroupNode, RootNode>{
public:
  RootNode( InstantiatedModule const & im)
  : Visitable{im}{}
  std::string name() const override;
};


