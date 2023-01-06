

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
  if (typeid(*expr) == typeid(Lookup)) {
    isLookup = true;
    const Lookup *lookup = static_cast<Lookup *>(expr);
    name = lookup->get_name();
  } else {
    isLookup = false;
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
  if (isLookup) {
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

struct SimplifiedExpression {
  const Expression *expression;
  boost::optional<ContextHandle<Context>> new_context = boost::none;
  boost::optional<const FunctionCall *> new_active_function_call = boost::none;
};
typedef std::variant<SimplifiedExpression, Value> SimplificationResult;

static SimplificationResult simplify_function_body(
const Expression *expression, const std::shared_ptr<const Context>& context)
{
  if (!expression) {
    return Value::undefined.clone();
  } else {
    const auto& type = typeid(*expression);
    if (type == typeid(TernaryOp)) {
      const TernaryOp *ternary = static_cast<const TernaryOp *>(expression);
      return SimplifiedExpression{ternary->evaluateStep(context)};
    } else if (type == typeid(Assert)) {
      const Assert *assertion = static_cast<const Assert *>(expression);
      return SimplifiedExpression{assertion->evaluateStep(context)};
    } else if (type == typeid(Echo)) {
      const Echo *echo = static_cast<const Echo *>(expression);
      return SimplifiedExpression{echo->evaluateStep(context)};
    } else if (type == typeid(Let)) {
      const Let *let = static_cast<const Let *>(expression);
      ContextHandle<Context> let_context{Context::create<Context>(context)};
      let_context->apply_config_variables(*context);
      return SimplifiedExpression{let->evaluateStep(let_context), std::move(let_context)};
    } else if (type == typeid(FunctionCall)) {
      const FunctionCall *call = static_cast<const FunctionCall *>(expression);

      const Expression *function_body;
      const AssignmentList *required_parameters;
      std::shared_ptr<const Context> defining_context;

      auto f = call->evaluate_function_expression(context);
      if (!f) {
        return Value::undefined.clone();
      } else {
        auto index = f->index();
        if (index == 0) {
            return std::get<const BuiltinFunction *>(*f)->evaluate(context, call);
        } else if (index == 1) {
            CallableUserFunction callable = std::get<CallableUserFunction>(*f);
            function_body = callable.function->expr.get();
            required_parameters = &callable.function->parameters;
            defining_context = callable.defining_context;
        } else {
          const FunctionType* function;
          if (index == 2) {
            function = &std::get<Value>(*f).toFunction();
          } else if (index == 3) {
            function = &std::get<const Value *>(*f)->toFunction();
          } else {
            assert(false);
          }
          function_body = function->getExpr().get();
          required_parameters = function->getParameters().get();
          defining_context = function->getContext();
        }
      }
      ContextHandle<Context> body_context{Context::create<Context>(defining_context)};
      body_context->apply_config_variables(*context);
      Arguments arguments{call->arguments, context};
      Parameters parameters = Parameters::parse(std::move(arguments), call->location(), *required_parameters, defining_context);
      body_context->apply_variables(std::move(parameters).to_context_frame());

      return SimplifiedExpression{function_body, std::move(body_context), call};
    } else {
      return expression->evaluate(context);
    }
  }
}

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
          LOG(message_group::Error, expression->location(), expression_context->documentRoot(), "Recursion detected calling function '%1$s'", current_call->name);
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

Expression *FunctionCall::create(const std::string& funcname, const AssignmentList& arglist, Expression *expr, const Location& loc)
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

