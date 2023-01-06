#include <core/expression/TernaryOp.h>
#include <core/Value.h>

TernaryOp::TernaryOp(Expression *cond, Expression *ifexpr, Expression *elseexpr, const Location& loc)
  : Expression(Id::TernaryOp,loc), cond(cond), ifexpr(ifexpr), elseexpr(elseexpr)
{
}

const Expression *TernaryOp::evaluateStep(const std::shared_ptr<const Context>& context) const
{
  return this->cond->evaluate(context).toBool() ? this->ifexpr.get() : this->elseexpr.get();
}

Value TernaryOp::evaluate(const std::shared_ptr<const Context>& context) const
{
  return evaluateStep(context)->evaluate(context);
}

void TernaryOp::print(std::ostream& stream, const std::string&) const
{
  stream << "(" << *this->cond << " ? " << *this->ifexpr << " : " << *this->elseexpr << ")";
}
