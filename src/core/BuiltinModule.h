#pragma once

#include <functional>

#include "AbstractModule.h"
#include "Arguments.h"
#include "Children.h"

class BuiltinModule : public AbstractModule
{
public:

   using fnContextInstantiate =
      abstractNodePtr(*)(ModInst const *, contextPtr const &);
   using fnArgsChildrenInstantiate =
      abstractNodePtr (*) (ModInst const *, Arguments, Children);

   BuiltinModule(fnContextInstantiate,Feature const *feature = nullptr);
   BuiltinModule(fnArgsChildrenInstantiate,Feature const *feature = nullptr);
   abstractNodePtr instantiate(
      contextPtr const & defining_context,
      ModInst const *inst,
      contextPtr const & context
   ) const override;

private:
  std::function<
    abstractNodePtr (ModInst const *, contextPtr const &)
  > do_instantiate;
};

