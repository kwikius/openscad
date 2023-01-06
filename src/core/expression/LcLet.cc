
#include <core/expression/Let.h>
#include <core/expression/LcLet.h>
#include <core/Value.h>


LcLet::LcLet(const AssignmentList& args, Expression *expr, const Location& loc)
  : ListComprehension(loc), arguments(args), expr(expr)
{
}

Value LcLet::evaluate(const std::shared_ptr<const Context>& context) const
{
  return this->expr->evaluate(*Let::sequentialAssignmentContext(this->arguments, this->location(), context));
}

void LcLet::print(std::ostream& stream, const std::string&) const
{
  stream << "let(" << this->arguments << ") (" << *this->expr << ")";
}
