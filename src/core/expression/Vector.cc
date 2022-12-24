
//#include <utils/boost-utils.h>

#include <core/Value.h>
#include <core/Context.h>

#include "Vector.h"

Vector::Vector(const Location& loc)
: Expression(Id::Vector,loc), literal_flag(boost::logic::indeterminate)
{
}

bool Vector::isLiteral() const {
  //
  if (boost::logic::indeterminate(literal_flag)) {
    for (const auto& e : this->children) {
      if (!e->isLiteral()) {
        literal_flag = false;
        return false;
      }
    }
    literal_flag = true;
    return true;
  } else {
    return bool(literal_flag);
  }
}

void Vector::emplace_back(Expression *expr)
{
  this->children.emplace_back(expr);
}

Value Vector::evaluate(const std::shared_ptr<const Context>& context) const
{
  if (children.size() == 1) {
    Value val = children.front()->evaluate(context);
    // If only 1 EmbeddedVectorType, convert to plain VectorType
    if (val.type() == Value::Type::EMBEDDED_VECTOR) {
      return VectorType(std::move(val.toEmbeddedVectorNonConst()));
    } else {
      VectorType vec(context->session());
      vec.emplace_back(std::move(val));
      return std::move(vec);
    }
  } else {
    VectorType vec(context->session());
    for (const auto& e : this->children) vec.emplace_back(e->evaluate(context));
    return std::move(vec);
  }
}

void Vector::print(std::ostream& stream, const std::string&) const
{
  stream << "[";
  for (size_t i = 0; i < this->children.size(); ++i) {
    if (i > 0) stream << ", ";
    stream << *this->children[i];
  }
  stream << "]";
}
