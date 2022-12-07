#pragma once

#include "../Expression.h"

class MemberLookup : public Expression{
public:
  MemberLookup(Expression *expr, const std::string& member, const Location& loc);
  Value evaluate(const std::shared_ptr<const Context>& context) const override;
  void print(std::ostream& stream, const std::string& indent) const override;
private:
  std::shared_ptr<Expression> expr;
  std::string member;
};
