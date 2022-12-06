#pragma once

#include "ListComprehension.h"
#include "../AssignmentList.h"

class LcLet : public ListComprehension
{
public:
  LcLet(const AssignmentList& args, Expression *expr, const Location& loc);
  Value evaluate(const std::shared_ptr<const Context>& context) const override;
  void print(std::ostream& stream, const std::string& indent) const override;
private:
  AssignmentList arguments;
  shared_ptr<Expression> expr;
};
