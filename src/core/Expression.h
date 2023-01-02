#pragma once

#include "AST.h"

class Value;
class Context;

class Expression : public ASTNode{
public:

  enum class Id {
     ArrayLookup,
     Assert,
     BinaryOp,
     Echo,
     FunctionCall,
     FunctionDefinition,
     Let,
     ListComprehension,
     Literal,
     Lookup,
     MemberLookup,
     ModuleLiteral,
     Range,
     TernaryOp,
     UnaryOp,
     ValueWrapper,
     Vector
  };

  virtual ~Expression() {}
  [[nodiscard]] virtual bool isLiteral() const { return false;}
  [[nodiscard]] Id getID() const { return m_id;}
  [[nodiscard]] virtual Value evaluate(const std::shared_ptr<const Context>& context) const = 0;
  [[nodiscard]] Value checkUndef(Value&& val, const std::shared_ptr<const Context>& context) const;
protected:
    Expression(Id id, const Location& loc) : ASTNode(loc), m_id(id) {}
 private :
     Id const m_id;
};

