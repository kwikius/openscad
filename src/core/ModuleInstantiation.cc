
/*
Copyright (C) Andy Little (kwikius@yahoo.com) 10/10/2022  initial revision module_reference evaluate
https://github.com/openscad/openscad/blob/master/COPYING
*/

#include <boost/filesystem.hpp>

#include <utils/exceptions.h>
#include <utils/printutils.h>
#include <utils/compiler_specific.h>

#include "module.h"
#include "Context.h"
#include "ModuleInstantiation.h"

namespace fs = boost::filesystem;

ModuleInstantiation::~ModuleInstantiation()
{
}

IfElseModuleInstantiation::~IfElseModuleInstantiation()
{
}

void ModuleInstantiation::print(std::ostream& stream, const std::string& indent, const bool inlined) const
{
  if (!inlined) stream << indent;
  stream << modname + "(";
  for (size_t i = 0; i < this->arguments.size(); ++i) {
    const auto& arg = this->arguments[i];
    if (i > 0) stream << ", ";
    if (!arg->getName().empty()) stream << arg->getName() << " = ";
    stream << *arg->getExpr();
  }
  if (scope.numElements() == 0) {
    stream << ");\n";
  } else if (scope.numElements() == 1) {
    stream << ") ";
    scope.print(stream, indent, true);
  } else {
    stream << ") {\n";
    scope.print(stream, indent + "\t", false);
    stream << indent << "}\n";
  }
}

void IfElseModuleInstantiation::print(std::ostream& stream, const std::string& indent, const bool inlined) const
{
  ModuleInstantiation::print(stream, indent, inlined);
  if (else_scope) {
    auto num_elements = else_scope->numElements();
    if (num_elements == 0) {
      stream << indent << "else;";
    } else {
      stream << indent << "else ";
      if (num_elements == 1) {
        else_scope->print(stream, indent, true);
      } else {
        stream << "{\n";
        else_scope->print(stream, indent + "\t", false);
        stream << indent << "}\n";
      }
    }
  }
}

/**
 * This is separated because PRINTB uses quite a lot of stack space
 * and the method using it evaluate()
 * is called often when recursive modules are evaluated.
 * noinline is required, as we here specifically optimize for stack usage
 * during normal operating, not runtime during error handling.
 */
static void NOINLINE print_trace(const ModuleInstantiation *mod, const std::shared_ptr<const Context> context){
  LOG(message_group::Trace, mod->location(), context->documentRoot(), "called by '%1$s'", mod->name());
}

std::shared_ptr<AbstractNode> ModuleInstantiation::evaluate(const std::shared_ptr<const Context> context) const
{
   std::string const old_name = this->modname;
   AssignmentList const old_args = this->arguments;

   auto setTo = [this](std::string const & name , AssignmentList const & args){
     const_cast<ModuleInstantiation*>(this)->modname = name;
     const_cast<ModuleInstantiation*>(this)->arguments = args;
   };

   std::shared_ptr<const Context> module_lookup_context = context;
   if ( id_expr) {
      auto const value = id_expr->evaluate(context);
      switch(value.type()){
         case Value::Type::MODULE:{
            auto const & modRef = value.toModuleReference();
            AssignmentList argsOut;
            if (modRef.transformToInstantiationArgs(
               this->arguments,
               this->loc,
               context,
               argsOut
            )){
               setTo(modRef.getModuleName(),argsOut);
               module_lookup_context = modRef.getContext();
            }else{
               setTo(old_name,old_args);
               return nullptr;
            }
         }
         break;
         default:
            LOG(message_group::Warning, this->loc, context->documentRoot(),
            "ModuleInstantiation: invalid id expression" );
         return nullptr;
      }
   }

   int32_t loopcount = 0;
   // max number of references to reference
   int32_t constexpr maxLoopCount = 1000;
   for(;;){
      if (++loopcount > maxLoopCount){
        LOG(message_group::Warning, this->loc, context->documentRoot(),
          "ModuleInstantiation: too many module_references '%1$s'", this->name());
        setTo(old_name,old_args);
        return nullptr;
      }
      boost::optional<InstantiableModule> module = module_lookup_context->lookup_module(this->name(), this->loc);
      if (module) {
        try{
          auto node = module->module->instantiate(module->defining_context, this, context);
          setTo(old_name,old_args);
          return node;
        } catch (EvaluationException& e) {
          setTo(old_name,old_args);
          if (e.traceDepth > 0) {
            print_trace(this, context);
            e.traceDepth--;
          }
          throw;
        }
     }else{
      boost::optional<const Value&> maybe_modRef =  module_lookup_context->lookup_moduleReference(this->name());
      if (!maybe_modRef ){
        LOG(message_group::Warning, this->loc, context->documentRoot(), "Ignoring unknown module/ref '%1$s'", this->name());
        setTo(old_name,old_args);
        return nullptr;
      }
      auto const & modRef = maybe_modRef->toModuleReference();

      AssignmentList argsOut;
      if (modRef.transformToInstantiationArgs(
         this->arguments,
         this->loc,
         context,
         argsOut
      )){
         setTo(modRef.getModuleName(),argsOut);
         module_lookup_context = modRef.getContext();
      }else{
          setTo(old_name,old_args);
          return nullptr;
      }
    }
  }
}

LocalScope *IfElseModuleInstantiation::makeElseScope()
{
  this->else_scope = std::make_unique<LocalScope>();
  return this->else_scope.get();
}
