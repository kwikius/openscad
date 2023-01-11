
#include <cmath>

#include <utils/printutils.h>

#include "Expression.h"
#include "LocalScope.h"
#include "UserModule.h"
#include "Value.h"
#include "ScopeContext.h"

void ScopeContext::init()
{
  for (const auto& assignment : scope->assignments) {
    if (assignment->getExpr()->isLiteral() && lookup_local_variable(assignment->getName())) {
      LOG(message_group::Warning, assignment->location(), this->documentRoot(),
        "Parameter %1$s is overwritten with a literal", assignment->getName());
    }
    try{
      set_variable(assignment->getName(), assignment->getExpr()->evaluate(get_shared_ptr()));
    } catch (EvaluationException& e) {
      if (e.traceDepth > 0) {
        if(assignment->locationOfOverwrite().isNone()){
          LOG(message_group::Trace, assignment->location(), this->documentRoot(),
            "assignment to '%1$s'", assignment->getName());
        } else {
          LOG(message_group::Trace, assignment->location(), this->documentRoot(),
             "overwritten assignment to '%1$s' (this is where the assignment is evaluated)", assignment->getName());
          LOG(message_group::Trace, assignment->locationOfOverwrite(), this->documentRoot(),
             "overwriting assignment to '%1$s'", assignment->getName());
        }
        e.traceDepth--;
      }
      throw;
    }
  }
}

boost::optional<CallableFunction>
ScopeContext::lookup_local_function(const std::string& name, const Location& loc) const
{
  const auto& search = scope->functions.find(name);
  if (search != scope->functions.end()) {
    return CallableFunction{CallableUserFunction{get_shared_ptr(), search->second.get()}};
  }
  return Context::lookup_local_function(name, loc);
}

boost::optional<InstantiableModule>
ScopeContext::lookup_local_module(const std::string& name, const Location& loc) const
{
  const auto& search = scope->modules.find(name);
  if (search != scope->modules.end()) {
    return InstantiableModule{get_shared_ptr(), search->second.get()};
  }
  return Context::lookup_local_module(name, loc);
}
