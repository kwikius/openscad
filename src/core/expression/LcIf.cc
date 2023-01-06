
#include <core/expression/LcIf.h>
#include <core/Value.h>

LcIf::LcIf(Expression *cond, Expression *ifexpr, Expression *elseexpr, const Location& loc)
  : ListComprehension(loc), cond(cond), ifexpr(ifexpr), elseexpr(elseexpr)
{
}

Value LcIf::evaluate(const std::shared_ptr<const Context>& context) const
{
  const std::shared_ptr<Expression>& expr = this->cond->evaluate(context).toBool() ? this->ifexpr : this->elseexpr;
  if (expr) {
    return expr->evaluate(context);
  } else {
    return EmbeddedVectorType::Empty();
  }
}

void LcIf::print(std::ostream& stream, const std::string&) const
{
  stream << "if(" << *this->cond << ") (" << *this->ifexpr << ")";
  if (this->elseexpr) {
    stream << " else (" << *this->elseexpr << ")";
  }
}
