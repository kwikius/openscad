#pragma once

#include <vector>

#include "AST.h"
#include "Expression.h"
#include "Assignment.h"
#include "LocalScope.h"

/**
* @brief object representing the required information for instantiation of a module
* or a module reference into the AST tree.
* The module is either represented as a name or as an expression returning a module literal
* TODO the module to instantiate represented as an abstract module_expression
* - module_expression
*    - module_name
*    - module_literal
*    - module_operation
*/
class ModuleInstantiation : public ASTNode
{
public:
  /**
   * @brief constructor used when the instantiation uses a module_expression
   * @param expr The module_expression to be instantiated
   * @param args The arguments to the instantiation
   * @param loc The sourcefile location
  */
  ModuleInstantiation(std::shared_ptr<Expression> expr, const AssignmentList& args , const Location& loc)
    : ASTNode(loc), arguments(args), tag_root(false), tag_highlight(false), tag_background(false), id_expr(expr) { }

 /**
   * @brief constructor used when the instantiation uses a name representing a module or module_reference
   * @param name The name of the module or module_reference to be instantiated
   * @param args The arguments to the instantiation
   * @param loc The sourcefile location
  */
  ModuleInstantiation(std::string const & name, const AssignmentList& args = AssignmentList(), const Location& loc = Location::NONE)
    : ASTNode(loc), arguments(args), tag_root(false), tag_highlight(false), tag_background(false), modname(name) { }
  ~ModuleInstantiation();

  /**
   * @brief output the instantiation params to the console
   */
  virtual void print(std::ostream& stream, const std::string& indent, const bool inlined) const;
  void print(std::ostream& stream, const std::string& indent) const override { print(stream, indent, false); }
  /**
   * @brief evaluate the instantiation. IOW instantiate it int the CSG tree
   */
  std::shared_ptr<AbstractNode> evaluate(std::shared_ptr<const Context> const & context) const;

  /**
   * name of the module to be instantiated in the source code
   * Note that static_cast<bool>(id_expr) == true then the name is not valid then the name is found via the id_expr
  */
  const std::string& name() const { return this->modname; }
  bool isBackground() const { return this->tag_background; }
  bool isHighlight() const { return this->tag_highlight; }
  bool isRoot() const { return this->tag_root; }

  std::shared_ptr<Expression> const & getIdExpr() const { return id_expr;}
  void setName(std::string const & name) { modname = name;}

  AssignmentList arguments;
  LocalScope scope;

  bool tag_root;
  bool tag_highlight;
  bool tag_background;
protected:
  std::string modname;
  std::shared_ptr<Expression> id_expr;
private:
    std::shared_ptr<AbstractNode> ll_evaluate(
    std::shared_ptr<const Context> const & context,
    std::shared_ptr<const Context> & module_lookup_context) const;
};

class IfElseModuleInstantiation : public ModuleInstantiation
{
public:
  IfElseModuleInstantiation(std::shared_ptr<class Expression> expr, const Location& loc) :
    ModuleInstantiation("if", AssignmentList{assignment("", expr)}, loc) { }
  ~IfElseModuleInstantiation();
  LocalScope *makeElseScope();
  LocalScope *getElseScope() const { return this->else_scope.get(); }
  void print(std::ostream& stream, const std::string& indent, const bool inlined) const final;
private:
  std::unique_ptr<LocalScope> else_scope;
};
