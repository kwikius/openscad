#pragma once

#include "../Expression.h"
#include "../Assignment.h"

class Assert final : public Expression
{
public:
  Assert(const AssignmentList& args, Expression *expr, const Location& loc);
  static void performAssert(const AssignmentList& arguments,
     const Location& location, const std::shared_ptr<const Context>& context);
  const Expression *evaluateStep(const std::shared_ptr<const Context>& context) const;
  Value evaluate(const std::shared_ptr<const Context>& context) const override;
  void print(std::ostream& stream, const std::string& indent) const override;
private:
  AssignmentList arguments;
  std::shared_ptr<Expression> expr;
};
