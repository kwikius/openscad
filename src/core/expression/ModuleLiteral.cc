
/*
Copyright (C) Andy Little (kwikius@yahoo.com) 10/10/2022  initial revision
https://github.com/openscad/openscad/blob/master/COPYING
*/
#include <cstdio>
#include <stack>
#include <algorithm>

#include <Feature.h>

#include "../ModuleReference.h"
#include "../LocalScope.h"
#include "../UserModule.h"

#include "ModuleLiteral.h"
#include "Literal.h"
#include "ValueWrapper.h"

//extern std::stack<LocalScope *> scope_stack;

Expression* MakeModuleLiteral(
   const std::string& moduleName,
   const AssignmentList& parameters,
   const AssignmentList& arguments,
   const Location& loc
)
{
   if (Feature::ExperimentalModuleLiteral.is_enabled()){
      return new ModuleLiteral(moduleName,parameters,arguments,loc);
   } else {
       LOG(message_group::Warning, loc,"", "Experimental module-literal is not enabled");
       return new Literal(loc);
   }
}

namespace {
  int32_t anonymousModuleNameCount = 0;
  char itoaArr[50] = {0};

  std::stack<std::string> anonModuleStack;
}

void ResetAnonymousModuleNameCount()
{
   anonymousModuleNameCount = 0;
}
std::string generateAnonymousModuleName()
{
   sprintf(itoaArr,"__&ML[%d]__",anonymousModuleNameCount++);
   return std::string(itoaArr);
}

void pushAnonymousModuleName(std::string const & name)
{
   anonModuleStack.push(name);
}

std::string popAnonymousModuleName()
{
   std::string name = anonModuleStack.top();
   anonModuleStack.pop();
   return name;
}

ModuleLiteral::ModuleLiteral(const std::string& mod_name, const AssignmentList &literal_params,
                         const AssignmentList& mod_args, const Location& loc )
: Expression(Id::ModuleLiteral,loc),
module_name(mod_name),
module_literal_parameters(literal_params),
module_arguments(mod_args)
{};

Value ModuleLiteral::evaluate(const std::shared_ptr<const Context>& context) const
{
   AssignmentList const & params_in = this->module_literal_parameters;
   AssignmentList * params_out = new AssignmentList;

   // put any evaluated default args in params_out
   for ( auto i = 0; i < params_in.size(); ++i){
      auto const & param = params_in[i];
      auto new_param = std::make_shared<Assignment>(param->getName(),loc);
      if ( static_cast<bool>(param->getExpr()) == true ){
         new_param->setExpr(
            std::make_shared<ValueWrapper>(
               std::move(param->getExpr()->evaluate(context)),
               location()
            )
         );
      }
      params_out->emplace_back(new_param);
   }
   // if no module_literal_params evaluate here
   // or ...
   // could also be done for module with parameters with more effort :
   // for each expression in the module_arguments
   // if it doesnt access a parameter then evaluate it
   // How to find if the module_argument accesses a module_parameter?
   //  walk the expressions in the module arguments, looking for module_param_names
   // which match the names of params in module_literal_params
   // expr->contains(param_names)
   // if found dont evaluate the expression

   AssignmentList * outArgs = new AssignmentList{module_arguments};
   if ( params_in.size() == 0){
      for ( auto i = 0; i < outArgs->size(); ++i){
          auto & arg = outArgs->at(i);
          //TODO if ! arg->getExpr().contains(params_in){...
          arg->setExpr(
            std::make_shared<ValueWrapper>(
               std::move(arg->getExpr()->evaluate(context)),
               location()
            )
         );
      }
   }

   return ModuleReferencePtr(
      ModuleReference(
         context,
         std::move(std::unique_ptr<AssignmentList>{params_out}),
         this->module_name,
         std::move(std::unique_ptr<AssignmentList>{outArgs})
      )
   );
}

void ModuleLiteral::print(std::ostream& stream, const std::string& indent) const
{
   stream << indent << "module ";
   if ( ! this->module_literal_parameters.empty()){
      stream << "(" << this->module_literal_parameters << ")";
   }
   stream << this->module_name ;
   if ( ! this->module_arguments.empty() ){
      stream << "(" << this->module_arguments << ")" ;
   }
}

