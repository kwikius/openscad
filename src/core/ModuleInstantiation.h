#pragma once

#include <string>
#include <vector>

#include "AST.h"
#include "Expression.h"
#include "Assignment.h"
#include "LocalScope.h"

/**
* @brief object representing the required information for instantiation of a module
* or a module reference into the AST tree.
* The module is either represented as
* - a named module instantiation
* - a module_expression instantiation
* - an if-else module instantiation
* TODO the module to instantiate represented as an abstract module_expression
* - module_expression
*    - module_name
*    - module_literal
*    - module_operation
*/
class NodeParams;
class ModuleInstantiation;
class ABCModuleInstantiation : public ASTNode{
protected:

 /**
   * @brief constructor used when the instantiation uses a name representing a module or module_reference
   * @param name The name of the module or module_reference to be instantiated
   * @param args The arguments to the instantiation
   * @param loc The sourcefile location
  */
  ABCModuleInstantiation(const AssignmentList& args = AssignmentList(), const Location& loc = Location::NONE)
    : ASTNode(loc), arguments(args), tag_root(false), tag_highlight(false), tag_background(false){ }

public:
  virtual ~ABCModuleInstantiation();
  /**
   * @brief output the instantiation params to the console
   */
  virtual void print(std::ostream& stream, const std::string& indent, const bool inlined) const =0;
  void print(std::ostream& stream, const std::string& indent) const override { print(stream, indent, false); }
  /**
   * @brief evaluate the instantiation. IOW instantiate it int the CSG tree
   */
  [[nodiscard]] virtual std::shared_ptr<AbstractNode>
  evalInst(std::shared_ptr<const Context> const & context) const =0;
  [[nodiscard]] bool isBackground() const { return this->tag_background; }
  [[nodiscard]] bool isHighlight() const { return this->tag_highlight; }
  [[nodiscard]] bool isRoot() const { return this->tag_root; }

  void setRoot() { tag_root = true;}
  void setHighlight() { tag_highlight = true;}
  void setBackground(){ tag_background = true;}

  /**
  * @todo Does this need to be a full scope?
  * Aren't the only entities it can contain child modules?
  **/
  [[nodiscard]] LocalScope const & getScope()const { return scope;}
  [[nodiscard]] LocalScope & getScopeNC() { return scope;}

  [[nodiscard]] AssignmentList const & getAssignmentList() const
  { return arguments;}

protected:

  [[nodiscard]] AssignmentList & getAssignmentListNC()
  { return arguments;}
   void setAssignmentList(AssignmentList && args)
  { arguments = args;}
    void print_scope_args(std::ostream& stream, const std::string& indent, const bool inlined)const;
   [[nodiscard]] static  bool
   evalModuleExpr(ModuleInstantiation* modInst,std::shared_ptr<Expression> const & expr,
     std::shared_ptr<const Context> & context);
   [[nodiscard]] static std::shared_ptr<ModuleInstantiation>
   evalModuleExpr(std::shared_ptr<Expression> const & expr,std::shared_ptr<const Context> & context);
private:
  mutable AssignmentList arguments;
  LocalScope scope;
  bool tag_root;
  bool tag_highlight;
  bool tag_background;
};

class ModuleInstantiation;
class ExprModInst : public ABCModuleInstantiation{
public :
   ExprModInst( std::shared_ptr<Expression> const & expr,
   const AssignmentList& args = AssignmentList(),
   const Location& loc = Location::NONE)
  : ABCModuleInstantiation{args,loc}, id_expr{expr}
  {}
public:
  void print(std::ostream& stream, const std::string& indent, const bool inlined) const override;
  [[nodiscard]] std::shared_ptr<AbstractNode>
  evalInst(std::shared_ptr<const Context> const & context) const override;
  private:
  std::shared_ptr<Expression> id_expr;
  mutable std::vector<std::shared_ptr<ModuleInstantiation> > instStack;
};

class ModuleInstantiation : public ABCModuleInstantiation{
public:
  ModuleInstantiation( std::string const & name,
   const AssignmentList& args = AssignmentList(),
   const Location& loc = Location::NONE)
  : ABCModuleInstantiation{args,loc}, modname{name}
  {}
  explicit ModuleInstantiation( ExprModInst const & exprMi)
   : ABCModuleInstantiation{static_cast<ABCModuleInstantiation const &>(exprMi) }
  {
  }

  operator NodeParams() const;
  void print(std::ostream& stream, const std::string& indent, const bool inlined) const override;
  [[nodiscard]] std::shared_ptr<AbstractNode>
  evalInst(std::shared_ptr<const Context> const & context) const override;
  std::string const & name() const { return modname;}
  void setName(std::string const & name){ modname = name;}
  [[nodiscard]] std::shared_ptr<AbstractNode> ll_evaluate(
    std::shared_ptr<const Context> const & context,
    std::shared_ptr<const Context> & module_lookup_context) const;
  private:
  mutable std::string modname;
};

class IfElseModuleInstantiation : public ModuleInstantiation{
public:
  IfElseModuleInstantiation(std::shared_ptr<class Expression> expr, const Location& loc) :
    ModuleInstantiation("if", AssignmentList{std::make_shared<Assignment>("", expr)}, loc) { }
  ~IfElseModuleInstantiation();
  LocalScope *makeElseScope();
  LocalScope *getElseScope() const { return this->else_scope.get(); }
  void print(std::ostream& stream, const std::string& indent, const bool inlined) const final;
private:
  std::unique_ptr<LocalScope> else_scope;
};
