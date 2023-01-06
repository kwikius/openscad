
#include <utils/printutils.h>

#include <core/Value.h>
#include <core/Context.h>

#include <core/expression/LcFor.h>

LcFor::LcFor(const AssignmentList& args, Expression *expr, const Location& loc)
  : ListComprehension(loc), arguments(args), expr(expr)
{
}

static inline ContextHandle<Context>
forContext(const std::shared_ptr<const Context>& context, const std::string& name, Value value)
{
  ContextHandle<Context> innerContext{Context::create<Context>(context)};
  innerContext->set_variable(name, std::move(value));
  return innerContext;
}

static void doForEach(
  const AssignmentList& assignments,
  const Location& location,
  const std::function<void(const std::shared_ptr<const Context>&)>& operation,
  size_t assignment_index,
  const std::shared_ptr<const Context>& context
  ) {
  if (assignment_index >= assignments.size()) {
    operation(context);
    return;
  }
   const std::string& variable_name = assignments[assignment_index]->getName();
   Value variable_values = assignments[assignment_index]->getExpr()->evaluate(context);
   switch (variable_values.type()){
      case Value::Type::RANGE:{
         const RangeType& range = variable_values.toRange();
         uint32_t steps = range.numValues();
         if (steps >= 1000000) {
            LOG(message_group::Warning, location, context->documentRoot(),
            "Bad range parameter in for statement: too many elements (%1$lu)", steps);
         } else {
            for (double value : range) {
               doForEach(assignments, location, operation, assignment_index + 1,
               *forContext(context, variable_name, value)
               );
            }
         }
      }
      break;
      case Value::Type::VECTOR:{
         for (const auto& value : variable_values.toVector()) {
            doForEach(assignments, location, operation, assignment_index + 1,
            *forContext(context, variable_name, value.clone())
            );
         }
      }
      break;
      case Value::Type::OBJECT:{
         for (auto key : variable_values.toObject().keys()) {
            doForEach(assignments, location, operation, assignment_index + 1,
            *forContext(context, variable_name, key)
            );
         }
      }
      break;
      case Value::Type::STRING:{
         for (auto value : variable_values.toStrUtf8Wrapper()) {
            doForEach(assignments, location, operation, assignment_index + 1,
            *forContext(context, variable_name, Value(std::move(value)))
            );
         }
      }
      break;
      case Value::Type::UNDEFINED:{

      }
      break;
      default:{
         doForEach(assignments, location, operation, assignment_index + 1,
         *forContext(context, variable_name, std::move(variable_values))
         );
      }
      break;
   }
}

void LcFor::forEach(const AssignmentList& assignments,
   const Location& loc, const std::shared_ptr<const Context>& context,
      std::function<void(const std::shared_ptr<const Context>&)> operation)
{
  doForEach(assignments, loc, operation, 0, context);
}

Value LcFor::evaluate(const std::shared_ptr<const Context>& context) const
{
  EmbeddedVectorType vec(context->session());
  forEach(this->arguments, this->loc, context,
          [&vec, expression = expr.get()] (const std::shared_ptr<const Context>& iterationContext) {
    vec.emplace_back(expression->evaluate(iterationContext));
  }
          );
  return Value(std::move(vec));
}

void LcFor::print(std::ostream& stream, const std::string&) const
{
  stream << "for(" << this->arguments << ") (" << *this->expr << ")";
}


