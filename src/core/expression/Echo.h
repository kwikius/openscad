#pragma once

#include "../Expression.h"
#include "../Assignment.h"

class Echo : public Expression{
public:
  Echo(const AssignmentList& args, Expression *expr, const Location& loc);
  const Expression *evaluateStep(const std::shared_ptr<const Context>& context) const;
  Value evaluate(const std::shared_ptr<const Context>& context) const override;
  void print(std::ostream& stream, const std::string& indent) const override;
private:
  AssignmentList arguments;
  std::shared_ptr<Expression> expr;
};
