#pragma once

#include "../Expression.h"

class TernaryOp final : public Expression
{
public:
  TernaryOp(Expression *cond, Expression *ifexpr, Expression *elseexpr, const Location& loc);
  const Expression *evaluateStep(const std::shared_ptr<const Context>& context) const;
  Value evaluate(const std::shared_ptr<const Context>& context) const override;
  void print(std::ostream& stream, const std::string& indent) const override;
private:
  std::shared_ptr<Expression> cond;
  std::shared_ptr<Expression> ifexpr;
  std::shared_ptr<Expression> elseexpr;
};

