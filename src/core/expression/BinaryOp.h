#pragma once

#include "../Expression.h"

class BinaryOp final : public Expression
{
public:
  //Be aware these enums are used as array indices
  //so take care if modifying
  enum class Op {
    LogicalAnd =0,
    LogicalOr = 1,
    Exponent =2,
    Multiply=3,
    Divide=4,
    Modulo=5,
    Plus=6,
    Minus=7,
    Less=8,
    LessEqual=9,
    Greater=10,
    GreaterEqual=11,
    Equal=12,
    NotEqual=13,
    Translate=14,
    Rotate = 15
  };

  BinaryOp(Expression *left, Op op, Expression *right, const Location& loc);
  Value evaluate(const std::shared_ptr<const Context>& context) const override;
  void print(std::ostream& stream, const std::string& indent) const override;
  Op  getOpID() const { return op;}
  std::shared_ptr<Expression> const & getLeft() const { return left;}
  std::shared_ptr<Expression> const &  getRight() const { return right;}

  const char *opString() const;
private:
  Op op;
  std::shared_ptr<Expression> left;
  std::shared_ptr<Expression> right;
};

