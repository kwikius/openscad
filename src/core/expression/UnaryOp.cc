#include <core/expression/UnaryOp.h>
#include <core/Value.h>
#include <utils/exceptions.h>

UnaryOp::UnaryOp(UnaryOp::Op op, Expression *expr, const Location& loc)
: Expression(Id::UnaryOp,loc), op(op), expr(expr)
{
}

Value UnaryOp::evaluate(const std::shared_ptr<const Context>& context) const
{
  switch (this->op) {
  case (Op::Not):    return !this->expr->evaluate(context).toBool();
  case (Op::Negate): return checkUndef(-this->expr->evaluate(context), context);
  default:
    assert(false && "Non-existent unary operator!");
    throw EvaluationException("Non-existent unary operator!");
  }
}

const char *UnaryOp::opString() const
{
  switch (this->op) {
  case Op::Not:    return "!";
  case Op::Negate: return "-";
  default:
    assert(false && "Non-existent unary operator!");
    throw EvaluationException("Non-existent unary operator!");
  }
}

bool UnaryOp::isLiteral() const {
  return this->expr->isLiteral();
}

void UnaryOp::print(std::ostream& stream, const std::string&) const
{
  stream << opString() << *this->expr;
}
