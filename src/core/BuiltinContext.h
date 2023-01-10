#pragma once

#include <string>

#include <boost/optional.hpp>

#include "Context.h"

class EvaluationSession;
class InstantiableModule;

class BuiltinContext : public Context{
public:
  void init() override;
  boost::optional<CallableFunction>
    lookup_local_function(const std::string& name, const Location& loc) const override;
  boost::optional<InstantiableModule>
    lookup_local_module(const std::string& name, const Location& loc) const override;

private:
  BuiltinContext(EvaluationSession *session);

  friend class Context;
};
