
#include <utils/printutils.h>
#include <core/expression/Echo.h>
#include <core/Value.h>
#include <core/Arguments.h>

Echo::Echo(const AssignmentList& args, Expression *expr, const Location& loc)
  : Expression(Id::Echo,loc), arguments(args), expr(expr)
{}

const Expression *Echo::evaluateStep(const std::shared_ptr<const Context>& context) const
{
  Arguments arguments{this->arguments, context};
  LOG(message_group::Echo, Location::NONE, "", "%1$s", STR(arguments));
  return expr.get();
}

Value Echo::evaluate(const std::shared_ptr<const Context>& context) const
{
  const Expression *nextexpr = evaluateStep(context);
  return nextexpr ? nextexpr->evaluate(context) : Value::undefined.clone();
}

void Echo::print(std::ostream& stream, const std::string&) const
{
  stream << "echo(" << this->arguments << ")";
  if (this->expr) stream << " " << *this->expr;
}
