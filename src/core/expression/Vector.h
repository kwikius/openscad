#pragma once

#include <vector>
#include <boost/logic/tribool.hpp>

#include "../Expression.h"

class Vector final : public Expression
{
public:
  Vector(const Location& loc);
  const std::vector<std::shared_ptr<Expression>>& getChildren() const { return children; }
  Value evaluate(const std::shared_ptr<const Context>& context) const override;
  void print(std::ostream& stream, const std::string& indent) const override;
  void emplace_back(Expression *expr);
  bool isLiteral() const override;
private:
  std::vector<std::shared_ptr<Expression>> children;
  mutable boost::tribool literal_flag; // cache if already computed
};

