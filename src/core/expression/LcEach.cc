
#include <utils/printutils.h>
#include <core/expression/LcEach.h>
#include <core/Value.h>
#include <core/Context.h>

LcEach::LcEach(Expression *expr, const Location& loc) : ListComprehension(loc), expr(expr)
{
}

// Need this for recurring into already embedded vectors, and performing "each" on their elements
//    Context is only passed along for the possible use in Range warning.
Value LcEach::evalRecur(Value&& v, const std::shared_ptr<const Context>& context) const
{
  if (v.type() == Value::Type::RANGE) {
    const RangeType& range = v.toRange();
    uint32_t steps = range.numValues();
    if (steps >= 1000000) {
      LOG(message_group::Warning, loc, context->documentRoot(), "Bad range parameter in for statement: too many elements (%1$lu)", steps);
    } else {
      EmbeddedVectorType vec(context->session());
      for (double d : range) vec.emplace_back(d);
      return Value(std::move(vec));
    }
  } else if (v.type() == Value::Type::VECTOR) {
    // Safe to move the overall vector ptr since we have a temporary value (could be a copy, or constructed just for us, doesn't matter)
    auto vec = EmbeddedVectorType(std::move(v.toVectorNonConst()));
    return Value(std::move(vec));
  } else if (v.type() == Value::Type::EMBEDDED_VECTOR) {
    EmbeddedVectorType vec(context->session());
    // Not safe to move values out of a vector, since it's shared_ptr maye be shared with another Value,
    // which should remain constant
    for (const auto& val : v.toEmbeddedVector()) vec.emplace_back(evalRecur(val.clone(), context) );
    return Value(std::move(vec));
  } else if (v.type() == Value::Type::STRING) {
    EmbeddedVectorType vec(context->session());
    for (auto ch : v.toStrUtf8Wrapper()) vec.emplace_back(std::move(ch));
    return Value(std::move(vec));
  } else if (v.type() != Value::Type::UNDEFINED) {
    return std::move(v);
  }
  return EmbeddedVectorType::Empty();
}

Value LcEach::evaluate(const std::shared_ptr<const Context>& context) const
{
  return evalRecur(this->expr->evaluate(context), context);
}

void LcEach::print(std::ostream& stream, const std::string&) const
{
  stream << "each (" << *this->expr << ")";
}
