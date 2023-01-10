
#include "Parameters.h"
#include "Arguments.h"
#include "UserModule.h"
#include "UserModuleContext.h"
#include "Value.h"

UserModuleContext::UserModuleContext(const std::shared_ptr<const Context> parent,
    const UserModule *module, const Location& loc, Arguments arguments, Children children) :
  ScopeContext(parent, &module->body),
  children(std::move(children))
{
  set_variable("$children", Value(double(this->children.size())));
  set_variable("$parent_modules", Value(double(StaticModuleNameStack::size())));
  apply_variables(Parameters::parse(std::move(arguments), loc, module->parameters, parent).to_context_frame());
}

std::vector<const std::shared_ptr<const Context> *> UserModuleContext::list_referenced_contexts() const
{
  std::vector<const std::shared_ptr<const Context> *> output = Context::list_referenced_contexts();
  output.push_back(&children.getContext());
  return output;
}
