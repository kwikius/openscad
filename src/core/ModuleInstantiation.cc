
/*
Copyright (C) Andy Little (kwikius@yahoo.com) 10/10/2022  initial revision module_reference evaluate
https://github.com/openscad/openscad/blob/master/COPYING
*/

#include <boost/filesystem.hpp>

#include <utils/exceptions.h>
#include <utils/printutils.h>
#include <utils/compiler_specific.h>

#include "AbstractModule.h"
#include "InstantiableModule.h"
#include "Context.h"
#include "ModuleInstantiation.h"
#include "Value.h"
#include <core/expression/ModuleLiteral.h>
#include <core/expression/BinaryOp.h>

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
  if ( id_expr){
   // stream << "( ";
    id_expr->print(stream,"");
   // stream << " )";
  }else{
     stream << modname;
  }
  stream <<  "(";
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

namespace{

   /**
   *  @brief expand a module expression into a module instantaiation using an existing moduleInstantatiation
      @param modInst The existing ModuleInstantiation that has been called on the ModuleExpression.
      @param expr The moduleExpression to expand
      @param context The context of the instantiation, which may be modified during the expansion
      @returns true if the Module expression was expanded successfully else false
   **/
   bool evalModuleExpr(ModuleInstantiation* modInst,
      std::shared_ptr<Expression> const & expr,std::shared_ptr<const Context> & context);


     /**
   *  @brief expand a module expression into a module instantiation using an new moduleInstantatiation
      @param expr The moduleExpression to expand
      @param context The context of the instantiation, which may be modified during the expansion
      @returns shared_ptr to the new ModuleInstantation if the Module expression was expanded successfully
      else empty shared_ptr
   **/
   std::shared_ptr<ModuleInstantiation>
   evalModuleExpr(std::shared_ptr<Expression> const & expr,std::shared_ptr<const Context> & context)
   {
      auto modInst = std::make_shared<ModuleInstantiation>("",AssignmentList(),expr->location());
      bool res = evalModuleExpr(modInst.get(),expr,context);
      if ( res){
         return modInst;
      }else{
         return std::shared_ptr<ModuleInstantiation>();
      }
   }

   bool evalModuleExpr(ModuleInstantiation* modInst,
      std::shared_ptr<Expression> const & expr,std::shared_ptr<const Context> & context)
   {
      if ( auto bin_op = std::dynamic_pointer_cast<BinaryOp>(expr)){
       using Op = BinaryOp::Op;
       switch ( bin_op->getOpID()) {
         case Op::Translate:
         case Op::Rotate:{
               // TODO check that modInst args is empty
               // and modInst has no child modules
               // the arg is already checked later in the builtin_translate etc fun
               auto arg_expr = bin_op->getRight();
               auto arg = std::make_shared<Assignment>("",arg_expr,arg_expr->location() );
               modInst->arguments.emplace_back(arg);
               std::shared_ptr<const Context> child_context = context;
               auto childModInst = evalModuleExpr(bin_op->getLeft(),child_context);
               modInst->setName(
                  (bin_op->getOpID() == Op::Translate)
                 ?"translate"
                 :"rotate"
               );
               modInst->scope.addModuleInst(childModInst);
               return true;
         }
/*
TODO
         case Group:
         case Union:
         case Intersection:
         case Difference: {
              lhsModInst = evalModuleExpr(bin_op->left);
              rhsModInst = evalModuleExpr(bin_op->right);
              assert(lhsModInst && rhsModInst);
              modInst.name = op.name;
              modInst.scope.addModInst(lhsModInst);
              modInst.scope.addModInst(rhsModInst);
              return true;
         }
*/
         default:{
           LOG(message_group::Warning, modInst->location(), context->documentRoot(),"invalid op");
           return false;
        }
      }
    }else{
            auto const value = expr->evaluate(context);
            switch(value.type()){
               case Value::Type::MODULE:{
                  auto const & modRef = value.toModuleReference();
                  AssignmentList argsOut;
                  if (modRef.transformToInstantiationArgs(
                     modInst->arguments,
                     modInst->location(),
                     context,
                     argsOut
                  )){
                     modInst->setName(modRef.getModuleName());
                     modInst->arguments = argsOut;
                     context = modRef.getContext();
                  }else{
                     return false;
                  }
               }
               break;
               default:
                  LOG(message_group::Warning, modInst->location(), context->documentRoot(),
                  "ModuleInstantiation: invalid id expression" );
               return false;
            }
         return true;
      }
   }
}// ~namespace

/**
*  ModuleInstantiation
**/
std::shared_ptr<AbstractNode>
ModuleInstantiation::evaluate(const std::shared_ptr<const Context> context) const
{
   // save all the parts that may be modified
   // restore them after the evaluation, thus preserving "constness" of the ModuleInstantiation
   std::string const old_name = this->modname;
   AssignmentList const old_args = this->arguments;
   std::vector<std::shared_ptr<ModuleInstantiation> > const old_children = this->scope.moduleInstantiations;
   //TODO LocalScope -> AbstractScope
   auto setTo = [this](std::string const & name , AssignmentList const & args){
     const_cast<ModuleInstantiation*>(this)->modname = name;
     const_cast<ModuleInstantiation*>(this)->arguments = args;
   };

   auto restore = [this,old_name,old_args,old_children](){
      const_cast<ModuleInstantiation*>(this)->modname = old_name;
      const_cast<ModuleInstantiation*>(this)->arguments = old_args;
      const_cast<ModuleInstantiation*>(this)->scope.moduleInstantiations = old_children;
   };

   std::shared_ptr<const Context> module_lookup_context = context;
   if ( id_expr) {
      bool res = evalModuleExpr(const_cast<ModuleInstantiation*>(this),id_expr,module_lookup_context);
      if ( ! res){
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
        restore();
        return nullptr;
      }
      boost::optional<InstantiableModule> maybe_module = module_lookup_context->lookup_module(this->name(), this->loc);
      if (maybe_module) {
        try{
          auto node = maybe_module->module->instantiate(maybe_module->defining_context, this, context);
          restore();
          return node;
        } catch (EvaluationException& e) {
          restore();
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
        restore();
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
          restore();
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
