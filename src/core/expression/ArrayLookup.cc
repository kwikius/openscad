
#include <core/expression/ArrayLookup.h>
#include <core/Value.h>

ArrayLookup::ArrayLookup(Expression *array, Expression *index, const Location& loc)
  : Expression(Id::ArrayLookup,loc), array(array), index(index)
{
}

Value ArrayLookup::evaluate(const std::shared_ptr<const Context>& context) const {
  return this->array->evaluate(context)[this->index->evaluate(context)];
}

void ArrayLookup::print(std::ostream& stream, const std::string&) const
{
  stream << *array << "[" << *index << "]";
}
