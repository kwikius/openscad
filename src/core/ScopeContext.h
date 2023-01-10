#pragma once

#include <memory>

#include "Context.h"

class LocalScope;

class ScopeContext : public Context
{
public:
  void init() override;
  boost::optional<CallableFunction> lookup_local_function(const std::string& name, const Location& loc) const override;
  boost::optional<InstantiableModule> lookup_local_module(const std::string& name, const Location& loc) const override;

protected:
  ScopeContext(const std::shared_ptr<const Context> parent, const LocalScope *scope) :
    Context(parent),
    scope(scope)
  {}

private:
// Experimental code. See issue #399
//	void evaluateAssignments(const AssignmentList &assignments);

  const LocalScope *scope;

  friend class Context;
};
