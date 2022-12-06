#pragma once

#include "ListComprehension.h"
#include "../Assignment.h"

class LcForC : public ListComprehension
{
public:
  LcForC(const AssignmentList& args, const AssignmentList& incrargs, Expression *cond, Expression *expr, const Location& loc);
  Value evaluate(const std::shared_ptr<const Context>& context) const override;
  void print(std::ostream& stream, const std::string& indent) const override;
private:
  AssignmentList arguments;
  AssignmentList incr_arguments;
  shared_ptr<Expression> cond;
  shared_ptr<Expression> expr;
};
