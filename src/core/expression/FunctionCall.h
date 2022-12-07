#pragma once

#include <boost/optional.hpp>

#include "../Expression.h"
#include "../Assignment.h"

class FunctionCall : public Expression
{
public:
  FunctionCall(Expression *expr, const AssignmentList& arglist, const Location& loc);
  boost::optional<CallableFunction> evaluate_function_expression(const std::shared_ptr<const Context>& context) const;
  Value evaluate(const std::shared_ptr<const Context>& context) const override;
  void print(std::ostream& stream, const std::string& indent) const override;
  const std::string& get_name() const { return name; }
  static Expression *create(const std::string& funcname, const AssignmentList& arglist, Expression *expr, const Location& loc);
public:
  bool isLookup;
  std::string name;
  std::shared_ptr<Expression> expr;
  AssignmentList arguments;
};
