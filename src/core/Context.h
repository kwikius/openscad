#pragma once

#include "ContextHandle.h"

/**
*  @brief Context.
* https://stackoverflow.com/a/6156770
* "Context refers to the execution context, which is the symbols
* reachable from a given point in the code,
* and the value of those symbols in that particular execution"
**/
class Context : public ContextFrame, public std::enable_shared_from_this<Context>{
protected:
  Context(EvaluationSession *session);
  Context(const std::shared_ptr<const Context>& parent);

public:
  ~Context() override;

  template <typename  C, typename ... Args>
     requires std::is_base_of_v<Context,C>
  static ContextHandle<C> create(Args&& ... args) {
    return ContextHandle<C>{std::shared_ptr<C>(new C(std::forward<Args>(args)...))};
  }

  virtual void init() { }

  std::shared_ptr<const Context> get_shared_ptr() const { return shared_from_this(); }
  virtual const class Children *user_module_children() const;
  virtual std::vector<const std::shared_ptr<const Context> *> list_referenced_contexts() const;

  boost::optional<const Value&> try_lookup_variable(const std::string& name) const;
  const Value& get_variable_value(const std::string& name, const Location& loc) const;
  boost::optional<CallableFunction> lookup_function(const std::string& name, const Location& loc) const;
  boost::optional<InstantiableModule> lookup_module(const std::string& name, const Location& loc) const;
  boost::optional<const Value&> lookup_moduleReference(const std::string& name) const;
  bool set_variable(const std::string& name, Value&& value) override;
  size_t clear() override;

  const std::shared_ptr<const Context>& getParent() const { return this->parent; }
  // This modifies the semantics of the context in an error-prone way. Use with caution.
  void setParent(const std::shared_ptr<const Context>& parent) { this->parent = parent; }

  void setAccountingAdded() { accountingAdded = true; }

protected:
  std::shared_ptr<const Context> parent;

  bool accountingAdded = false;   // avoiding bad accounting when exception threw in constructor issue #3871

public:
#ifdef DEBUG
  std::string dump() const;
#endif
};
