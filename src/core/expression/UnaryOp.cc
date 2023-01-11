#include <core/expression/UnaryOp.h>
#include <core/Value.h>
#include <core/Context.h>
#include <utils/exceptions.h>

UnaryOp::UnaryOp(UnaryOp::Op op, Expression *expr, const Location& loc)
: Expression(Id::UnaryOp,loc), op(op), expr(expr)
{
}

Value UnaryOp::evaluate(const std::shared_ptr<const Context>& context) const
{
  auto const & location = this->loc;
  auto checkUndef = [context,location](Value&& val){
    if (val.isUncheckedUndef()) LOG(message_group::Warning, location, context->documentRoot(),
      "%1$s", val.toUndefString());
    return std::move(val);
  };
  Value const val = this->expr->evaluate(context);
  switch (this->op) {
  case (Op::Not):    return !val.toBool();
  case (Op::Negate): return checkUndef(-val);
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
