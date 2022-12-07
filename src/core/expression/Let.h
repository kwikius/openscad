#pragma once

#include "../Expression.h"
#include "../Assignment.h"

class Let : public Expression{
public:
  Let(const AssignmentList& args, Expression *expr, const Location& loc);
  static void doSequentialAssignment(const AssignmentList& assignments,
     const Location& location, ContextHandle<Context>& targetContext);
  static ContextHandle<Context> sequentialAssignmentContext(
     const AssignmentList& assignments, const Location& location,
     const std::shared_ptr<const Context>& context);
  const Expression *evaluateStep(ContextHandle<Context>& targetContext) const;
  Value evaluate(const std::shared_ptr<const Context>& context) const override;
  void print(std::ostream& stream, const std::string& indent) const override;
private:
  AssignmentList arguments;
  std::shared_ptr<Expression> expr;
};
