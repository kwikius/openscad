
#include <utils/printutils.h>
#include <utils/exceptions.h>
#include <core/Parameters.h>
#include <core/Arguments.h>
#include <core/Value.h>
#include <core/Context.h>

#include <core/expression/Assert.h>

Assert::Assert(const AssignmentList& args, Expression *expr, const Location& loc)
  : Expression(Id::Assert,loc), arguments(args), expr(expr)
{}

void Assert::performAssert(const AssignmentList& arguments, const Location& location, const std::shared_ptr<const Context>& context)
{
  Parameters parameters = Parameters::parse(Arguments(arguments, context), location, {"condition"}, {"message"});
  const Expression *conditionExpression = nullptr;
  for (const auto& argument : arguments) {
    if (argument->getName() == "" || argument->getName() == "condition") {
      conditionExpression = argument->getExpr().get();
      break;
    }
  }

  if (!parameters["condition"].toBool()) {
    std::string conditionString = conditionExpression ? STR(" '", *conditionExpression, "'") : "";
    std::string messageString = parameters.contains("message") ? (": " + parameters["message"].toEchoStringNoThrow()) : "";
    LOG(message_group::Error, location, context->documentRoot(), "Assertion%1$s failed%2$s", conditionString, messageString);
    throw AssertionFailedException("Assertion Failed", location);
  }
}

const Expression *Assert::evaluateStep(const std::shared_ptr<const Context>& context) const
{
  performAssert(this->arguments, this->loc, context);
  return expr.get();
}

Value Assert::evaluate(const std::shared_ptr<const Context>& context) const
{
  const Expression *nextexpr = evaluateStep(context);
  return nextexpr ? nextexpr->evaluate(context) : Value::undefined.clone();
}

void Assert::print(std::ostream& stream, const std::string&) const
{
  stream << "assert(" << this->arguments << ")";
  if (this->expr) stream << " " << *this->expr;
}
