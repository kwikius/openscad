#pragma once

#include <functional>

#include "AbstractModule.h"
#include "Arguments.h"
#include "Children.h"

/**
* @brief BuiltinModules have 2 forms.
* In the first (primitives), they have arguments but no children.
* in the second (transforms) they have arguments and children.
**/
class BuiltinModule : public AbstractModule
{
public:

   using fnContextInstantiate =
      abstractNodePtr(*)(ModInst const *, contextPtr const &);
   using fnArgsChildrenInstantiate =
      abstractNodePtr (*) (ModInst const *, Arguments, Children const & );

   BuiltinModule(fnContextInstantiate,Feature const *feature = nullptr);
   BuiltinModule(fnArgsChildrenInstantiate,Feature const *feature = nullptr);

  /**
   * @brief instantiate override for BuiltinModule
   * @param defining_context Ignored since the module is builtin
   * @param inst Holds argumnents and children, if any
   * @param context The context where the module was instantiated
   **/
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

