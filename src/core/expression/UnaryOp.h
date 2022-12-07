#pragma once

#include "../Expression.h"

class UnaryOp : public Expression
{
public:
  enum class Op {
    Not,
    Negate
  };
  bool isLiteral() const override;
  UnaryOp(Op op, Expression *expr, const Location& loc);
  Value evaluate(const std::shared_ptr<const Context>& context) const override;
  void print(std::ostream& stream, const std::string& indent) const override;

private:
  const char *opString() const;

  Op op;
  std::shared_ptr<Expression> expr;
};
