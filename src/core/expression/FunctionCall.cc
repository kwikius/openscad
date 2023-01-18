

#include <utils/StackCheck.h>
#include <utils/printutils.h>
#include <utils/compiler_specific.h>
#include <core/Context.h>
#include <core/Arguments.h>
#include <core/Parameters.h>
#include <core/Value.h>

#include <core/expression/FunctionCall.h>
#include <core/expression/Echo.h>
#include <core/expression/Let.h>
#include <core/expression/Assert.h>
#include <core/expression/Lookup.h>
#include <core/expression/TernaryOp.h>

/**
 * This is separated because PRINTB uses quite a lot of stack space
 * and the method using it evaluate()
 * is called often when recursive functions are evaluated.
 * noinline is required, as we here specifically optimize for stack usage
 * during normal operating, not runtime during error handling.
 */
static void NOINLINE print_err(const char *name, const Location& loc, const std::shared_ptr<const Context>& context){
  LOG(message_group::Error, loc, context->documentRoot(), "Recursion detected calling function '%1$s'", name);
}

/**
 * This is separated because PRINTB uses quite a lot of stack space
 * and the method using it evaluate()
 * is called often when recursive functions are evaluated.
 * noinline is required, as we here specifically optimize for stack usage
 * during normal operating, not runtime during error handling.
 */
static void NOINLINE print_trace(const FunctionCall *val, const std::shared_ptr<const Context>& context){
  LOG(message_group::Trace, val->location(), context->documentRoot(), "called by '%1$s'", val->get_name());
}

FunctionCall::FunctionCall(Expression *expr, const AssignmentList& args, const Location& loc)
  : Expression(Id::FunctionCall,loc), expr(expr), arguments(args)
{
   if ( this->isLookup = expr->getID() == Id::Lookup){
      this->name = static_cast<Lookup const &>(*expr).get_name();
   } else {
      std::ostringstream s;
      s << "(";
      expr->print(s, "");
      s << ")";
      name = s.str();
   }
}

boost::optional<CallableFunction>
FunctionCall::evaluate_function_expression(const std::shared_ptr<const Context>& context) const
{
  if (this->isLookup) {
    return context->lookup_function(name, location());
  } else {
    auto v = expr->evaluate(context);
    if (v.type() == Value::Type::FUNCTION) {
      return CallableFunction{std::move(v)};
    } else {
      LOG(message_group::Warning, loc, context->documentRoot(), "Can't call function on %1$s", v.typeName());
      return boost::none;
    }
  }
}
namespace {
   struct SimplifiedExpression {
     const Expression *expression;
     boost::optional<ContextHandle<Context>> new_context = boost::none;
     boost::optional<const FunctionCall *> new_active_function_call = boost::none;
   };
   typedef std::variant<SimplifiedExpression, Value> SimplificationResult;

   static SimplificationResult simplify_function_body(
   const Expression *expression, const std::shared_ptr<const Context>& context)
   {
      if (expression != nullptr) {
         auto evaluateStep = [expression,context]<typename T>(){
            return SimplifiedExpression{
               static_cast<T const&>(*expression).evaluateStep(context)
            };
         };
         switch( expression->getID()){
            using Id = Expression::Id;
            case Id ::TernaryOp:
               return evaluateStep.operator()<TernaryOp>();
            case Id::Assert:
               return evaluateStep.operator()<Assert>();
            case Id::Echo:
               return evaluateStep.operator()<Echo>();
            case Id::Let:{
               ContextHandle<Context> let_context{Context::create<Context>(context)};
               let_context->apply_config_variables(*context);
               return SimplifiedExpression{
                  static_cast<Let const&>(*expression).evaluateStep(let_context),
                  std::move(let_context)
               };
            }
            case Id::FunctionCall:{
               auto const * call = static_cast<FunctionCall const *>(expression);
               if (auto const f = call->evaluate_function_expression(context)) {  // F is an optional CallableFunction
                  const Expression *function_body;
                  const AssignmentList *required_parameters;
                  std::shared_ptr<const Context> defining_context;
                  // f = CallableFunction = std::variant<const BuiltinFunction *, CallableUserFunction, Value, const Value *>;
                  switch (auto index = f->index()){
                     case 0:  // BuiltinFunction *
                        return std::get<const BuiltinFunction *>(*f)->evaluate(context, call);
                     case 1:{  // CallableUserFunction
                        CallableUserFunction callable = std::get<CallableUserFunction>(*f);
                        function_body = callable.function->expr.get();
                        required_parameters = &callable.function->parameters;
                        defining_context = callable.defining_context;
                        break;
                     }
                     case 2:  // Value
                     case 3: { // Value const *
                        const FunctionType* function =
                        (index == 2)
                           ? &std::get<Value>(*f).toFunction()
                           : &std::get<const Value *>(*f)->toFunction();
                        function_body = function->getExpr().get();
                        required_parameters = function->getParameters().get();
                        defining_context = function->getContext();
                        break;
                     }
                     default:{ // shouldnt get here!
                        assert(false);
                     }
                  }// ~switch f->index
                  ContextHandle<Context> body_context{Context::create<Context>(defining_context)};
                  body_context->apply_config_variables(*context);
                  Arguments arguments{call->arguments, context};
                  Parameters parameters = Parameters::parse(std::move(arguments), call->location(),
                     *required_parameters, defining_context
                  );
                  body_context->apply_variables(std::move(parameters).to_context_frame());
                  return SimplifiedExpression{function_body, std::move(body_context), call};
               }else{
                  return Value::undefined.clone();
               }
            }
            default:{
               return expression->evaluate(context);
            }
        }
     }else { // expression == nullptr
       return Value::undefined.clone();
     }
   }
}//~namespace

Value FunctionCall::evaluate(const std::shared_ptr<const Context>& context) const
{
  const auto& name = get_name();
  if (StackCheck::inst().check()) {
    print_err(name.c_str(), loc, context);
    throw RecursionException::create("function", name, this->loc);
  }

  // Repeatedly simplify expr until it reduces to either a tail call,
  // or an expression that cannot be simplified in-place. If the latter,
  // recurse. If the former, substitute the function body for expr,
  // thereby implementing tail recursion optimization.
  unsigned int recursion_depth = 0;
  const FunctionCall *current_call = this;

  ContextHandle<Context> expression_context{Context::create<Context>(context)};
  const Expression *expression = this;
  while (true) {
    try {
      auto result = simplify_function_body(expression, *expression_context);
      if (Value *value = std::get_if<Value>(&result)) {
        return std::move(*value);
      }

      SimplifiedExpression *simplified_expression = std::get_if<SimplifiedExpression>(&result);
      assert(simplified_expression);

      expression = simplified_expression->expression;
      if (simplified_expression->new_context) {
        expression_context = std::move(*simplified_expression->new_context);
      }
      if (simplified_expression->new_active_function_call) {
        current_call = *simplified_expression->new_active_function_call;
        if (recursion_depth++ == 1000000) {
          LOG(message_group::Error, expression->location(), expression_context->documentRoot(),
             "Recursion detected calling function '%1$s'", current_call->name);
          throw RecursionException::create("function", current_call->name, current_call->location());
        }
      }
    } catch (EvaluationException& e) {
      if (e.traceDepth > 0) {
        print_trace(current_call, *expression_context);
        e.traceDepth--;
      }
      throw;
    }
  }
}

void FunctionCall::print(std::ostream& stream, const std::string&) const
{
  stream << this->get_name() << "(" << this->arguments << ")";
}

Expression *FunctionCall::create(const std::string& funcname, const AssignmentList& arglist,
   Expression *expr, const Location& loc)
{
  if (funcname == "assert") {
    return new Assert(arglist, expr, loc);
  } else if (funcname == "echo") {
    return new Echo(arglist, expr, loc);
  } else if (funcname == "let") {
    return new Let(arglist, expr, loc);
  }
  return nullptr;
  // TODO: Generate error/warning if expr != 0?
  //return new FunctionCall(funcname, arglist, loc);
}

