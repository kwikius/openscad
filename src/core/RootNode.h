#pragma once
#include <string>

#include "AbstractNode.h"
/*!
   Only instantiated once, for the top-level file.
 */
class RootNode final : public Visitable<GroupNode, RootNode>{
public:
  RootNode( NodeParams const & np)
  : Visitable{np}{}
  std::string name() const override;
};


