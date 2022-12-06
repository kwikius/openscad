#pragma once

#include "../Expression.h"

class ArrayLookup : public Expression
{
public:
  ArrayLookup(Expression *array, Expression *index, const Location& loc);
  Value evaluate(const std::shared_ptr<const Context>& context) const override;
  void print(std::ostream& stream, const std::string& indent) const override;
private:
  std::shared_ptr<Expression> array;
  std::shared_ptr<Expression> index;
};

