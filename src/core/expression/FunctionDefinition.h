#pragma once

#include "../Expression.h"
#include "../Assignment.h"

class FunctionDefinition : public Expression{
public:
  FunctionDefinition(Expression *expr, const AssignmentList& parameters, const Location& loc);
  Value evaluate(const std::shared_ptr<const Context>& context) const override;
  void print(std::ostream& stream, const std::string& indent) const override;
public:
  std::shared_ptr<const Context> context;
  AssignmentList parameters;
  std::shared_ptr<Expression> expr;
};

