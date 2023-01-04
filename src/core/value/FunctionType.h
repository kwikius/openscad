#pragma once

#include <memory>
#include <iostream>

class FunctionType
{
public:
  FunctionType(std::shared_ptr<const Context> context,
  std::shared_ptr<Expression> expr, std::shared_ptr<AssignmentList> parameters)
    : context(context), expr(expr), parameters(parameters) { }
  Value operator==(const FunctionType& other) const;
  Value operator!=(const FunctionType& other) const;
  Value operator<(const FunctionType& other) const;
  Value operator>(const FunctionType& other) const;
  Value operator<=(const FunctionType& other) const;
  Value operator>=(const FunctionType& other) const;

  const std::shared_ptr<const Context>& getContext() const { return context; }
  const std::shared_ptr<Expression>& getExpr() const { return expr; }
  const std::shared_ptr<AssignmentList>& getParameters() const { return parameters; }
private:
  std::shared_ptr<const Context> context;
  std::shared_ptr<Expression> expr;
  std::shared_ptr<AssignmentList> parameters;
};

std::ostream& operator<<(std::ostream& stream, const FunctionType& f);
