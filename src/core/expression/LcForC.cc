
#include <utils/printutils.h>
#include <core/expression/LcForC.h>
#include <core/Value.h>
#include <core/Context.h>
#include <core/expression/Let.h>

LcForC::LcForC(const AssignmentList& args, const AssignmentList& incrargs, Expression *cond,
    Expression *expr, const Location& loc)
  : ListComprehension(loc), arguments(args), incr_arguments(incrargs), cond(cond), expr(expr)
{
}

Value LcForC::evaluate(const std::shared_ptr<const Context>& context) const
{
  EmbeddedVectorType output(context->session());

  ContextHandle<Context> initialContext{Let::sequentialAssignmentContext(this->arguments, this->location(), context)};
  ContextHandle<Context> currentContext{Context::create<Context>(*initialContext)};

  unsigned int counter = 0;
  while (this->cond->evaluate(*currentContext).toBool()) {
    output.emplace_back(this->expr->evaluate(*currentContext));

    if (counter++ == 1000000) {
      LOG(message_group::Error, loc, context->documentRoot(), "For loop counter exceeded limit");
      throw LoopCntException::create("for", loc);
    }

    /**
     * @brief The next context should be evaluated in the current context,
     * and replace the current context; but there is no reason for
     * it to _parent_ the current context, for the next context
     * replaces every variable in it. Keeping the next context
     * parented to the current context would keep the current in
     * memory unnecessarily, and greatly slow down variable lookup.
     * However, we can't just use apply_variables(), as this breaks
     * captured context references in lambda functions.
     * So, we reparent the next context to the initial context.
     **/
    ContextHandle<Context> nextContext{
      Let::sequentialAssignmentContext(this->incr_arguments, this->location(), *currentContext)
    };
    currentContext = std::move(nextContext);
    currentContext->setParent(*initialContext);
  }
  return Value(std::move(output));
}

void LcForC::print(std::ostream& stream, const std::string&) const
{
  stream
    << "for(" << this->arguments
    << ";" << *this->cond
    << ";" << this->incr_arguments
    << ") " << *this->expr;
}
