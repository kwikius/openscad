#pragma once

#include <ostream>
#include <string>
#include <vector>

#include "AST.h"
#include "Expression.h"
#include "Annotation.h"

class Assignment : public ASTNode
{
public:
  Assignment(std::string name, const Location& loc)
    : ASTNode(loc), name(name), locOfOverwrite(Location::NONE) { }
  Assignment(std::string name,
             std::shared_ptr<class Expression> expr = std::shared_ptr<class Expression>(),
             const Location& loc = Location::NONE)
    : ASTNode(loc), name(name), expr(expr), locOfOverwrite(Location::NONE){ }

  void print(std::ostream& stream, const std::string& indent) const override;
  const std::string& getName() const { return name; }
  const std::shared_ptr<Expression>& getExpr() const { return expr; }
  const AnnotationMap& getAnnotations() const { return annotations; }
  // setExpr used by customizer ParameterObject etc.
  void setExpr(std::shared_ptr<Expression> e) { expr = std::move(e); }
  void setName( const std::string & nameIn) { name = nameIn;}
  bool hasName() const { return name !="";}
  void resetExpr() { expr.reset();}
  bool hasExpr() const { return static_cast<bool>(expr);}
  virtual void addAnnotations(AnnotationList *annotations);
  virtual bool hasAnnotations() const;
  virtual const Annotation *annotation(const std::string& name) const;

  const Location& locationOfOverwrite() const { return locOfOverwrite; }
  void setLocationOfOverwrite(const Location& locOfOverwrite) { this->locOfOverwrite = locOfOverwrite; }

protected:
  std::string name;
  std::shared_ptr<class Expression> expr;
  AnnotationMap annotations;
  Location locOfOverwrite;
};

template <class ... Args> std::shared_ptr<Assignment> assignment(Args... args) {
  return std::make_shared<Assignment>(args ...);
}

typedef std::vector<std::shared_ptr<Assignment>> AssignmentList;
typedef std::unordered_map<std::string, const Expression *> AssignmentMap;

std::ostream& operator<<(std::ostream& stream, const AssignmentList& assignments);
