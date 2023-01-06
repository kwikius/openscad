#pragma once

#include <string>
#include <utility>
#include <vector>

#include <boost/optional.hpp>

#include "ContextMemoryManager.h"
#include "function.h"
#include "InstantiableModule.h"

class Value;
class ContextFrame;

/**
*  @brief New instance of EvaluationSession created on the stack when the main file ( documentroot) is recompiled
*  Main member is a context stack.
*  The Evaluation session doesnt appear to own the Contexts on the stack.
*  Lifetime is somewhat obsure therefore
*  push_frame() and pop_frame() push and pop Context on the stack
*  EvaluationSession is created on the stack in MainWindow::instantiateRoot()
*  and in openscad.cc do_export
**/
class EvaluationSession
{
public:
  EvaluationSession(std::string documentRoot) :
    document_root(std::move(documentRoot))
  {}

  /**
  *  @brief push a new Context
  *  @return current context stack index ( stack is based on a std::vector of context*
  *  Note that EvaluationSession doesnt own the Context
  *
  **/
  size_t push_frame(ContextFrame *frame);
  /**
   * @brief
   **/
  void replace_frame(size_t index, ContextFrame *frame);
  void pop_frame(size_t index);

  [[nodiscard]] boost::optional<const Value&> try_lookup_special_variable(const std::string& name) const;
  [[nodiscard]] const Value& lookup_special_variable(const std::string& name, const Location& loc) const;
  [[nodiscard]] boost::optional<CallableFunction> lookup_special_function(const std::string& name, const Location& loc) const;
  [[nodiscard]] boost::optional<InstantiableModule> lookup_special_module(const std::string& name, const Location& loc) const;

  [[nodiscard]] const std::string& documentRoot() const { return document_root; }
  ContextMemoryManager& contextMemoryManager() { return context_memory_manager; }
  HeapSizeAccounting& accounting() { return context_memory_manager.accounting(); }

private:
  std::string document_root;
  std::vector<ContextFrame *> stack;
  ContextMemoryManager context_memory_manager;
};
