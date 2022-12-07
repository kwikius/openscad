#pragma once

#include <functional>
#include <string>
#include <variant>
#include <vector>

#include <Feature.h>

#include "AST.h"
#include "Assignment.h"
#include "Value.h"

class Arguments;
class FunctionCall;

class BuiltinFunction
{
public:
  std::function<Value(const std::shared_ptr<const Context>&, const FunctionCall *)> evaluate;

private:
  const Feature *feature;

public:
  BuiltinFunction(Value(*f)(const std::shared_ptr<const Context>&, const FunctionCall *), const Feature *feature = nullptr);
  BuiltinFunction(Value(*f)(Arguments, const Location&), const Feature *feature = nullptr);

  bool is_experimental() const { return feature != nullptr; }
  bool is_enabled() const { return (feature == nullptr) || feature->is_enabled(); }
};

class UserFunction : public ASTNode
{
public:
  std::string name;
  AssignmentList parameters;
  std::shared_ptr<Expression> expr;

  UserFunction(const char *name, AssignmentList& parameters,
   std::shared_ptr<Expression> expr, const Location& loc);

  void print(std::ostream& stream, const std::string& indent) const override;
};


struct CallableUserFunction
{
  std::shared_ptr<const Context> defining_context;
  const UserFunction *function;
};
typedef std::variant<const BuiltinFunction *, CallableUserFunction, Value, const Value *> CallableFunction;
