#pragma once

/*
Copyright (C) Andy Little (kwikius@yahoo.com) 10/10/2022  initial revision
https://github.com/openscad/openscad/blob/master/COPYING
*/

#include "../Expression.h"
#include "../Assignment.h"

class ModuleLiteral final : public Expression{
public:
  ModuleLiteral(const std::string& name, const AssignmentList & literal_params,
               const AssignmentList& module_args, const Location& loc );
  Value evaluate(const std::shared_ptr<const Context>& context) const override;
  void print(std::ostream& stream, const std::string& indent) const override;
  // the name of the module  we are referring to
  std::string const module_name;
  AssignmentList module_literal_parameters;
  AssignmentList module_arguments;
};

std::string generateAnonymousModuleName();

Expression* MakeModuleLiteral(
   const std::string& moduleName,  AssignmentList const & params,
   const AssignmentList& args, const Location& loc );



