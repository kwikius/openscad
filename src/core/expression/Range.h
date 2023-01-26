#pragma once

#include "../Expression.h"

class Range final : public Expression
{
public:
  Range(Expression *begin, Expression *end, const Location& loc);
  Range(Expression *begin, Expression *step, Expression *end, const Location& loc);
  const Expression *getBegin() const { return begin.get(); }
  const Expression *getStep() const { return step.get(); }
  const Expression *getEnd() const { return end.get(); }
  Value evaluate(const std::shared_ptr<const Context>& context) const override;
  void print(std::ostream& stream, const std::string& indent) const override;
  bool isLiteral() const override;
private:
  std::shared_ptr<Expression> begin;
  std::shared_ptr<Expression> step;
  std::shared_ptr<Expression> end;
};
