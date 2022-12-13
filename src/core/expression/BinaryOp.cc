
#include <cstdint>
#include <cassert>
#include <utils/printutils.h>
#include <utils/exceptions.h>
#include <core/Context.h>
#include <core/expression/BinaryOp.h>

BinaryOp::BinaryOp(Expression *left, BinaryOp::Op op, Expression *right, const Location& loc) :
  Expression(loc), op(op), left(left), right(right)
{
}

Value BinaryOp::evaluate(const std::shared_ptr<const Context>& context) const
{
  switch (this->op) {
  case Op::LogicalAnd:
    return this->left->evaluate(context).toBool() && this->right->evaluate(context).toBool();
  case Op::LogicalOr:
    return this->left->evaluate(context).toBool() || this->right->evaluate(context).toBool();
  case Op::Exponent:
    return checkUndef(this->left->evaluate(context) ^ this->right->evaluate(context), context);
  case Op::Multiply:
    return checkUndef(this->left->evaluate(context) * this->right->evaluate(context), context);
  case Op::Divide:
    return checkUndef(this->left->evaluate(context) / this->right->evaluate(context), context);
  case Op::Modulo:
    return checkUndef(this->left->evaluate(context) % this->right->evaluate(context), context);
  case Op::Plus:
    return checkUndef(this->left->evaluate(context) + this->right->evaluate(context), context);
  case Op::Minus:
    return checkUndef(this->left->evaluate(context) - this->right->evaluate(context), context);
  case Op::Less:
    return checkUndef(this->left->evaluate(context) < this->right->evaluate(context), context);
  case Op::LessEqual:
    return checkUndef(this->left->evaluate(context) <= this->right->evaluate(context), context);
  case Op::Greater:
    return checkUndef(this->left->evaluate(context) > this->right->evaluate(context), context);
  case Op::GreaterEqual:
    return checkUndef(this->left->evaluate(context) >= this->right->evaluate(context), context);
  case Op::Equal:
    return checkUndef(this->left->evaluate(context) == this->right->evaluate(context), context);
  case Op::NotEqual:
    return checkUndef(this->left->evaluate(context) != this->right->evaluate(context), context);
  case Op::Translate:
    LOG(message_group::Warning, loc, context->documentRoot(), "translate TODO");
    // return modref_translation(this)
    return Value::undefined.clone();
  case Op::Rotate:
    LOG(message_group::Warning, loc, context->documentRoot(), "rotate TODO");
    // return modref_rotation(this)
    return Value::undefined.clone();
  default:
    assert(false && "Non-existent binary operator!");
    throw EvaluationException("Non-existent binary operator!");
  }
}

const char *BinaryOp::opString() const
{
  switch (this->op) {
  case Op::LogicalAnd:   return "&&";
  case Op::LogicalOr:    return "||";
  case Op::Exponent:     return "^";
  case Op::Multiply:     return "*";
  case Op::Divide:       return "/";
  case Op::Modulo:       return "%";
  case Op::Plus:         return "+";
  case Op::Minus:        return "-";
  case Op::Less:         return "<";
  case Op::LessEqual:    return "<=";
  case Op::Greater:      return ">";
  case Op::GreaterEqual: return ">=";
  case Op::Equal:        return "==";
  case Op::NotEqual:     return "!=";
  case Op::Translate:    return "->";
  case Op::Rotate:       return "^>";
  default:
    assert(false && "Non-existent binary operator!");
    throw EvaluationException("Non-existent binary operator!");
  }
}

void BinaryOp::print(std::ostream& stream, const std::string&) const
{
  stream << "(" << *this->left << " " << opString() << " " << *this->right << ")";
}
