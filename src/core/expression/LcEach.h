#pragma once

#include "ListComprehension.h"

class LcEach : public ListComprehension
{
public:
  LcEach(Expression *expr, const Location& loc);
  Value evaluate(const std::shared_ptr<const Context>& context) const override;
  void print(std::ostream& stream, const std::string& indent) const override;
private:
  Value evalRecur(Value&& v, const std::shared_ptr<const Context>& context) const;
  std::shared_ptr<Expression> expr;
};
