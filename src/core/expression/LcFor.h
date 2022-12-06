#pragma once

#include "ListComprehension.h"
#include "../Assignment.h"

class LcFor : public ListComprehension
{
public:
  LcFor(const AssignmentList& args, Expression *expr, const Location& loc);
  static void forEach(const AssignmentList& assignments, const Location& loc,
    const std::shared_ptr<const Context>& context,
     std::function<void(const std::shared_ptr<const Context>&)> operation);
  Value evaluate(const std::shared_ptr<const Context>& context) const override;
  void print(std::ostream& stream, const std::string& indent) const override;
private:
  AssignmentList arguments;
  std::shared_ptr<Expression> expr;
};
