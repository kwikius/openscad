#pragma one

#include <core/Expression.h>
#include <core/Value.h>
/**
* @brief  a Literal is an expression that already evaluated to a Value.
* The member value can hold any type of Value, which may or may not be
* a numeric_constant.
**/
class Literal final : public Expression{
public:
  Literal(const Location& loc = Location::NONE)
  : Expression(Id::Literal,loc), value(Value::undefined.clone()) { };
  Literal(Value val, const Location& loc = Location::NONE)
  : Expression(Id::Literal,loc), value(std::move(val)) { };
  bool isBool() const { return value.type() == Value::Type::BOOL; }
  bool toBool() const { return value.toBool(); }
  bool isDouble() const { return value.type() == Value::Type::NUMBER; }
  double toDouble() const { return value.toDouble(); }
  bool isString() const { return value.type() == Value::Type::STRING; }
  const std::string& toString() const { return value.toStrUtf8Wrapper().toString(); }
  bool isUndefined() const { return value.type() == Value::Type::UNDEFINED; }

  Value evaluate(const std::shared_ptr<const Context>& context) const override;
  void print(std::ostream& stream, const std::string& indent) const override;
  bool isLiteral() const override { return true; }
private:
  const Value value;
};

