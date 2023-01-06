
#include <set>

#include <utils/printutils.h>
#include <core/expression/Let.h>
#include <core/Value.h>
#include <core/Context.h>

Let::Let(const AssignmentList& args, Expression *expr, const Location& loc)
  : Expression(Id::Let,loc), arguments(args), expr(expr)
{
}

void Let::doSequentialAssignment(const AssignmentList& assignments,
   const Location& location, ContextHandle<Context>& targetContext)
{
  std::set<std::string> seen;
  for (const auto& assignment : assignments) {
    Value value = assignment->getExpr()->evaluate(*targetContext);
    if (assignment->getName().empty()) {
      LOG(message_group::Warning, location, targetContext->documentRoot(),
         "Assignment without variable name %1$s", value.toEchoStringNoThrow());
    } else if (seen.find(assignment->getName()) != seen.end()) {
      LOG(message_group::Warning, location, targetContext->documentRoot(),
         "Ignoring duplicate variable assignment %1$s = %2$s", assignment->getName(), value.toEchoStringNoThrow());
    } else {
      targetContext->set_variable(assignment->getName(), std::move(value));
      seen.insert(assignment->getName());
    }
  }
}

ContextHandle<Context>
Let::sequentialAssignmentContext(const AssignmentList& assignments,
   const Location& location, const std::shared_ptr<const Context>& context)
{
  ContextHandle<Context> letContext{Context::create<Context>(context)};
  doSequentialAssignment(assignments, location, letContext);
  return letContext;
}

const Expression *Let::evaluateStep(ContextHandle<Context>& targetContext) const
{
  doSequentialAssignment(this->arguments, this->location(), targetContext);
  return this->expr.get();
}

Value Let::evaluate(const std::shared_ptr<const Context>& context) const
{
  ContextHandle<Context> letContext{Context::create<Context>(context)};
  return evaluateStep(letContext)->evaluate(*letContext);
}

void Let::print(std::ostream& stream, const std::string&) const
{
  stream << "let(" << this->arguments << ") " << *expr;
}
