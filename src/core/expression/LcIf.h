#pragma once

#include "ListComprehension.h"

class LcIf : public ListComprehension{
public:
  LcIf(Expression *cond, Expression *ifexpr, Expression *elseexpr, const Location& loc);
  Value evaluate(const std::shared_ptr<const Context>& context) const override;
  void print(std::ostream& stream, const std::string& indent) const override;
private:
  shared_ptr<Expression> cond;
  shared_ptr<Expression> ifexpr;
  shared_ptr<Expression> elseexpr;
};
