#pragma once

#include "../Expression.h"

class BinaryOp : public Expression
{
public:
  enum class Op {
    LogicalAnd,
    LogicalOr,
    Exponent,
    Multiply,
    Divide,
    Modulo,
    Plus,
    Minus,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    Equal,
    NotEqual,
    Translate,
    Rotate
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

