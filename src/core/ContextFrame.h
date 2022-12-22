#pragma once
#include <string>

#include "ValueMap.h"
#include "EvaluationSession.h"

/**
 * @brief base class for Context, also main member of Parameters class.
 *
 * Holds a pointer to the Evaluation session
 * as well as ValueMaps of config variables and lexical variables
 **/
class ContextFrame
{
public:
  ContextFrame(EvaluationSession *session);
  virtual ~ContextFrame() {}

  ContextFrame(ContextFrame&& other) = default;

  virtual boost::optional<const Value&> lookup_local_variable(const std::string& name) const;
  virtual boost::optional<CallableFunction> lookup_local_function(const std::string& name, const Location& loc) const;
  virtual boost::optional<InstantiableModule> lookup_local_module(const std::string& name, const Location& loc) const;

  virtual std::vector<const Value *> list_embedded_values() const;
  virtual size_t clear();

  /**
   * @brief add a variable to lexical or config variables
   * dependendent on its name decoration
   * @param name  The name to store the variable under.
   * @param value The value to store.
  **/
  virtual bool set_variable(const std::string& name, Value&& value);

  void apply_variables(const ValueMap& variables);
  void apply_lexical_variables(const ContextFrame& other);
  void apply_config_variables(const ContextFrame& other);
  void apply_variables(const ContextFrame& other) {
    apply_lexical_variables(other);
    apply_config_variables(other);
  }

  void apply_variables(ValueMap&& variables);
  void apply_lexical_variables(ContextFrame&& other);
  void apply_config_variables(ContextFrame&& other);
  void apply_variables(ContextFrame&& other);

  static bool is_config_variable(const std::string& name);

  EvaluationSession *session() const { return evaluation_session; }
  const std::string& documentRoot() const { return evaluation_session->documentRoot(); }

protected:
  ValueMap lexical_variables;
  ValueMap config_variables;
  EvaluationSession *evaluation_session;

public:
#ifdef DEBUG
  virtual std::string dumpFrame() const;
#endif
};


