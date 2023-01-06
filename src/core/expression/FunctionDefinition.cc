
#include <core/expression/FunctionDefinition.h>
#include <core/Value.h>

FunctionDefinition::FunctionDefinition(Expression *expr, const AssignmentList& parameters, const Location& loc)
  : Expression(Id::FunctionDefinition,loc), context(nullptr), parameters(parameters), expr(expr)
{
}

Value FunctionDefinition::evaluate(const std::shared_ptr<const Context>& context) const
{
  return FunctionPtr{FunctionType{context, expr, std::unique_ptr<AssignmentList>{new AssignmentList{parameters}}}};
}

void FunctionDefinition::print(std::ostream& stream, const std::string& indent) const
{
  stream << indent << "function(";
  bool first = true;
  for (const auto& parameter : parameters) {
    stream << (first ? "" : ", ") << parameter->getName();
    if (parameter->getExpr()) {
      stream << " = " << *parameter->getExpr();
    }
    first = false;
  }
  stream << ") " << *this->expr;
}
