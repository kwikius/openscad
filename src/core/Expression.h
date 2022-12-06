#pragma once

#include <list>

#include "AST.h"
#include "Value.h"

class Expression : public ASTNode{
public:
  Expression(const Location& loc) : ASTNode(loc) {}
  ~Expression() {}
  virtual bool isLiteral() const;
  virtual Value evaluate(const std::shared_ptr<const Context>& context) const = 0;
  Value checkUndef(Value&& val, const std::shared_ptr<const Context>& context) const;
};

using ExpressionList = std::list<std::shared_ptr<Expression> >;
