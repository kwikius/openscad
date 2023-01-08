#pragma once

#include <unordered_map>

#include "Assignment.h"

class AbstractNode;
class Context;
class ABCModuleInstantiation;
class UserModule;
class UserFunction;

/**
* @brief repository for storage of ModuleInstantiations, UserModule definitions and values
* Member of ABCModuleInstantiation, ScopeContext, UserModule, SourceFile,
* LocalScopes are pushed and popped from a parser stack during parsing, and entities are added
* as various entities are parsed.
**/
class LocalScope
{
public:
  size_t numElements() const { return assignments.size() + moduleInstantiations.size(); }
  void print(std::ostream& stream, const std::string& indent, const bool inlined = false) const;
  std::shared_ptr<AbstractNode> instantiateModules(
   const std::shared_ptr<const Context>& context, const std::shared_ptr<AbstractNode> &target) const;
  std::shared_ptr<AbstractNode> instantiateModules(
   const std::shared_ptr<const Context>& context, const std::shared_ptr<AbstractNode> &target,
     const std::vector<size_t>& indices) const;
  void addModuleInst(const std::shared_ptr<ABCModuleInstantiation>& modinst);
  void addModule(const std::shared_ptr<UserModule>& module);
  void addFunction(const std::shared_ptr<UserFunction>& function);
  void addAssignment(const std::shared_ptr<Assignment>& assignment);
  bool hasChildren() const {return !(moduleInstantiations.empty());}

  AssignmentList assignments;
  std::vector<std::shared_ptr<ABCModuleInstantiation> > moduleInstantiations;

  // Modules and functions are stored twice; once for lookup and once for AST serialization
  // FIXME: Should we split this class into an ASTNode and a run-time support class?
  std::unordered_map<std::string, std::shared_ptr<UserFunction>> functions;
  std::vector<std::pair<std::string, std::shared_ptr<UserFunction>>> astFunctions;

  std::unordered_map<std::string, std::shared_ptr<UserModule>> modules;
  std::vector<std::pair<std::string, std::shared_ptr<UserModule>>> astModules;
};
