
/*
Copyright (C) Andy Little (kwikius@yahoo.com) 10/10/2022  initial revision module_reference evaluate
https://github.com/openscad/openscad/blob/master/COPYING
*/
#include <stack>
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

IfElseModuleInstantiation::~IfElseModuleInstantiation()
{
}

ABCModuleInstantiation::~ABCModuleInstantiation()
{
}

void ABCModuleInstantiation::print_scope_args(std::ostream& stream, const std::string& indent, const bool inlined) const
{
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

void ModuleInstantiation::print(std::ostream& stream, const std::string& indent, const bool inlined) const
{
  if (!inlined) stream << indent;
  stream << this->modname;
  this->print_scope_args(stream,indent,inlined);
}

void ExprModInst::print(std::ostream& stream, const std::string& indent, const bool inlined) const
{
  if (!inlined) stream << indent;
  stream << "( ";
  this->id_expr->print(stream,"");
  stream << " )";
  this->print_scope_args(stream,indent,inlined);
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
   [[nodiscard]] bool evalModuleExpr(ModuleInstantiation* modInst,
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
/**
*  @brief Turn the expression into a module instantiation
*  @param modInst The ModuleInstantiation which will represent the resulting expression
*  @param expr The module expression to be converted
*  @return true if the expression was succesfully converted, then the result has been metamorphised
*  in modInst, else false
**/
   [[nodiscard]] bool evalModuleExpr(ModuleInstantiation* modInst,
      std::shared_ptr<Expression> const & expr,std::shared_ptr<const Context> & context)
   {
      switch( expr->getID()){
        using exprId = Expression::Id;
        case exprId::BinaryOp: {
          auto binOp = std::dynamic_pointer_cast<BinaryOp>(expr);
          assert(static_cast<bool>(binOp)==true);
          assert(modInst->arguments.empty());
          assert(modInst->name() == "");
          switch ( binOp->getOpID()) {
            using Op = BinaryOp::Op;
            case Op::Translate:
            case Op::Rotate:{
               auto arg_expr = binOp->getRight();
               auto arg = std::make_shared<Assignment>("",arg_expr,arg_expr->location() );
               modInst->arguments.emplace_back(arg);
               std::shared_ptr<const Context> child_context = context;
               auto childModInst = evalModuleExpr(binOp->getLeft(),child_context);
               if ( childModInst){
                  modInst->setName(
                     (binOp->getOpID() == Op::Translate)
                    ?"translate"
                    :"rotate"
                  );
                  modInst->scope.addModuleInst(childModInst);
                  return true;
               }else{// something went wrong. evalModuleExpr has provided the diagnostic
                  return false;
               }
            }// ~Op::Rotate Op::Translate
            case Op::Minus:{ // difference
               std::shared_ptr<const Context> child_context = context;
               auto lhsInst = evalModuleExpr(binOp->getLeft(),child_context);
               if ( ! lhsInst ){
                  return false;
               }
               child_context = context;
               auto rhsInst = evalModuleExpr(binOp->getRight(),child_context);
               if (! rhsInst){
                  return false;
               }
               modInst->setName("difference");
               modInst->scope.addModuleInst(lhsInst);
               modInst->scope.addModuleInst(rhsInst);
               return true;
            }// ~Op::Minus
   /*
   TODO
            case Union:         // '|'
            case LinearExtrude: // <->
            case RotateExtrude:  // <^>
            case Intersection:  // '&'
   */
           default:{ // Op::default
              LOG(message_group::Warning, modInst->location(), context->documentRoot(),"invalid op");
              return false;
           }
         } // ~switch binOp->getOpID()
       } // ~case exprId::BinaryOp
       default: {
         auto const value = expr->evaluate(context);
         if ( value.type() == Value::Type::MODULE){
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
               return true;
            }else{
               return false;
            }
         }else {
              LOG(message_group::Warning, modInst->location(), context->documentRoot(),
               "invalid module instantiation expression" );
              return false;
         }
        } // exprId::default
      } // ~switch expr->getID()
   } // ~()
}// ~namespace

std::shared_ptr<AbstractNode>
ModuleInstantiation::ll_evaluate(
   std::shared_ptr<const Context> const & context,
   std::shared_ptr<const Context> & module_lookup_context ) const
{
   std::string const old_name = this->modname;
   AssignmentList const old_args = this->arguments;
   //TODO LocalScope -> AbstractScope
   auto setTo = [this](std::string const & name , AssignmentList const & args){
      const_cast<ModuleInstantiation*>(this)->modname = name;
      const_cast<ModuleInstantiation*>(this)->arguments = args;
   };

   auto restore = [this,old_name,old_args](){
      const_cast<ModuleInstantiation*>(this)->modname = old_name;
      const_cast<ModuleInstantiation*>(this)->arguments = old_args;
   };

   // max number of references to reference
   int32_t constexpr maxLoopCount = 1000;
   for(int32_t loopCount = 0;loopCount < maxLoopCount;++loopCount){
      boost::optional<InstantiableModule> maybe_module
         = module_lookup_context->lookup_module(this->name(), this->loc);
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
         boost::optional<const Value&> maybe_modRef
           = module_lookup_context->lookup_moduleReference(this->name());
         if (!maybe_modRef ){
            LOG(message_group::Warning, this->loc, context->documentRoot(),
              "Ignoring unknown module/ref '%1$s'", this->name());
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
   LOG(message_group::Warning, this->loc, context->documentRoot(),
      "ModuleInstantiation: too many module_references '%1$s'", this->name());
   restore();
   return nullptr;
}

namespace {
  std::stack<std::shared_ptr<ModuleInstantiation> > modstack;
}
std::shared_ptr<AbstractNode>
ExprModInst::evalInst(std::shared_ptr<const Context> const & context) const
{
   std::shared_ptr<const Context> module_lookup_context = context;
   std::shared_ptr<ModuleInstantiation> mi;
   // Has the expression already been expanded?
   auto iter = this->instMap.find(context.get());
   // If not then expand it and put it in the map
   if (iter == std::end(instMap)){
//      LOG(message_group::Warning, this->loc, context->documentRoot(),
//      "adding mod expr to map");
      // N.B note this is a special converting constructor from ExprModInst to ModuleInstantiation
      // just slices off the useful parts
      mi = std::make_shared<ModuleInstantiation>(*this);
      if (evalModuleExpr(mi.get(),id_expr,module_lookup_context)){
         instMap.insert({context.get(),mi});
      }else{
        // evalModuleExpr has provided the diagnostic
        return nullptr;
      }
   }else{
      //already in map
      mi = iter->second;
   }
   return mi->ll_evaluate(context,module_lookup_context);
}

std::shared_ptr<AbstractNode>
ModuleInstantiation::evalInst(std::shared_ptr<const Context> const & context) const
{
   std::shared_ptr<const Context> module_lookup_context = context;
   return ll_evaluate(context,module_lookup_context);
}

LocalScope *IfElseModuleInstantiation::makeElseScope()
{
  this->else_scope = std::make_unique<LocalScope>();
  return this->else_scope.get();
}
