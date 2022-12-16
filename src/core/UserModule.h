#pragma once

#include <string>
#include <vector>

#include "AbstractModule.h"
#include "Assignment.h"
#include "LocalScope.h"

class StaticModuleNameStack
{
public:
  StaticModuleNameStack(const std::string& name) {
    stack.push_back(name);
  }
  ~StaticModuleNameStack() {
    stack.pop_back();
  }

  static int size() { return stack.size(); }
  static const std::string& at(int idx) { return stack[idx]; }

private:
  static std::vector<std::string> stack;
};
/**
* @brief Modules defined in source code
**/
class UserModule : public AbstractModule, public ASTNode
{
public:
  UserModule(const char *name, const Location& loc) : ASTNode(loc), name(name) { }
  UserModule(const char *name, const class Feature& feature, const Location& loc)
  : AbstractModule(feature), ASTNode(loc), name(name) { }
  ~UserModule() {}

  std::shared_ptr<AbstractNode> instantiate(
    contextPtr const & defining_context,
    ModInst const *inst,
    contextPtr const& context
  ) const override;
  void print(std::ostream& stream, const std::string& indent) const override;
  static const std::string& stack_element(int n) { return StaticModuleNameStack::at(n); }
  static int stack_size() { return StaticModuleNameStack::size(); }

  std::string name;
  AssignmentList parameters;
  LocalScope body;
};
