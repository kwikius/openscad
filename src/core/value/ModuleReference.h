
#pragma once

/*
Copyright (C) Andy Little (kwikius@yahoo.com) 10/10/2022  initial revision
https://github.com/openscad/openscad/blob/master/COPYING
*/

#include <cstdint>
#include <iostream>
#include <memory>

#include <core/Assignment.h>
#include "ValuePtr.h"

class Context;
class Expression;
class Value;

void ResetModuleReferenceUniqueID();

class ModuleReference{
   friend void ResetModuleReferenceUniqueID();
public:

  ModuleReference(std::shared_ptr<const Context> context_in,
                  std::shared_ptr<AssignmentList> literal_params_in,
                  std::string const & module_name_in,
                  std::shared_ptr<AssignmentList> mod_args_in
                  )
    : context(context_in),
      module_literal_parameters(literal_params_in),
      module_name(module_name_in),
      module_args(mod_args_in),
      m_unique_id(generate_unique_id())
      { }

  Value operator==(const ModuleReference& other) const;
  Value operator!=(const ModuleReference& other) const;
  Value operator<(const ModuleReference& other) const;
  Value operator>(const ModuleReference& other) const;
  Value operator<=(const ModuleReference& other) const;
  Value operator>=(const ModuleReference& other) const;

  const std::shared_ptr<const Context>& getContext() const { return context; }
  const std::shared_ptr<AssignmentList>& getModuleLiteralParameters() const { return module_literal_parameters; }
  const std::string & getModuleName() const { return module_name; }
  const std::shared_ptr<AssignmentList>& getModuleArgs() const { return module_args; }
  bool transformToInstantiationArgs(
      AssignmentList const & evalContextArgs,
      const Location& loc,
      const std::shared_ptr<const Context> evalContext,
      AssignmentList & argsOut
  ) const ;
  int64_t getUniqueID() const { return m_unique_id;}
private:
/*
  static bool processNamedArgs(
      std::shared_ptr<AssignmentList> const & paramsIn,
      std::shared_ptr<AssignmentList> const & Args,
      std::shared_ptr<AssignmentList> & paramsOut);
  bool processUnnamedArgs(
      std::shared_ptr<AssignmentList> const & paramsIn,
      std::shared_ptr<AssignmentList> const & Args,
      std::shared_ptr<AssignmentList> & paramsOut);
*/
  static int64_t generate_unique_id() { ++ next_id; return next_id;}
  static int64_t next_id;
  std::shared_ptr<const Context> context;
  std::shared_ptr<AssignmentList> module_literal_parameters;
  std::string module_name;
  std::shared_ptr<AssignmentList> module_args;
  int64_t const m_unique_id;
};

std::ostream& operator<<(std::ostream& stream, const ModuleReference& m);
