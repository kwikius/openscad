

#include <utils/compiler_specific.h>
#include <utils/printutils.h>
#include <core/Value.h>
#include <core/Context.h>
#include <core/expression/Range.h>

Range::Range(Expression *begin, Expression *end, const Location& loc)
  : Expression(Id::Range,loc), begin(begin), end(end)
{
}

Range::Range(Expression *begin, Expression *step, Expression *end, const Location& loc)
  : Expression(Id::Range,loc), begin(begin), step(step), end(end)
{
}

/**
 * This is separated because both PRINT_DEPRECATION and PRINT use
 * quite a lot of stack space and the method using it evaluate()
 * is called often when recursive functions are evaluated.
 * noinline is required, as we here specifically optimize for stack usage
 * during normal operating, not runtime during error handling.
 */
static void NOINLINE print_range_depr(const Location& loc, const std::shared_ptr<const Context>& context){
  std::string locs = loc.toRelativeString(context->documentRoot());
  LOG(message_group::Deprecated, loc, context->documentRoot(), "Using ranges of the form [begin:end] with begin value greater than the end value is deprecated");
}

static void NOINLINE print_range_err(const std::string& begin, const std::string& step, const Location& loc, const std::shared_ptr<const Context>& context){
  LOG(message_group::Warning, loc, context->documentRoot(), "begin %1$s than the end, but step %2$s", begin, step);
}

Value Range::evaluate(const std::shared_ptr<const Context>& context) const
{
  Value beginValue = this->begin->evaluate(context);
  if (beginValue.type() == Value::Type::NUMBER) {
    Value endValue = this->end->evaluate(context);
    if (endValue.type() == Value::Type::NUMBER) {
      double begin_val = beginValue.toDouble();
      double end_val = endValue.toDouble();

      if (!this->step) {
        if (end_val < begin_val) {
          std::swap(begin_val, end_val);
          print_range_depr(loc, context);
        }
        return RangeType(begin_val, end_val);
      } else {
        Value stepValue = this->step->evaluate(context);
        if (stepValue.type() == Value::Type::NUMBER) {
          double step_val = stepValue.toDouble();
          if (this->isLiteral()) {
            if ((step_val > 0) && (end_val < begin_val)) {
              print_range_err("is greater", "is positive", loc, context);
            } else if ((step_val < 0) && (end_val > begin_val)) {
              print_range_err("is smaller", "is negative", loc, context);
            }
          }
          return RangeType(begin_val, step_val, end_val);
        }
      }
    }
  }
  return Value::undefined.clone();
}

void Range::print(std::ostream& stream, const std::string&) const
{
  stream << "[" << *this->begin;
  if (this->step) stream << " : " << *this->step;
  stream << " : " << *this->end;
  stream << "]";
}

bool Range::isLiteral() const {
  return this->step ?
         begin->isLiteral() && end->isLiteral() && step->isLiteral() :
         begin->isLiteral() && end->isLiteral();
}
