#pragma once

#include "ScopeContext.h"
#include "Children.h"

class UserModule;

class UserModuleContext final : public ScopeContext
{
public:
  const Children *user_module_children() const override { return &children; }
  std::vector<const std::shared_ptr<const Context> *> list_referenced_contexts() const override;

private:
  UserModuleContext(const std::shared_ptr<const Context> parent,
    const UserModule *module, const Location& loc, Arguments arguments, Children children);

  Children children;

  friend class Context;
};
