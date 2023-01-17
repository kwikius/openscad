
#include <core/expression/Lookup.h>
#include <core/Value.h>
#include <core/Context.h>

Lookup::Lookup(const std::string& name, const Location& loc)
: Expression(Id::Lookup,loc), name(name)
{
}

Value Lookup::evaluate(const std::shared_ptr<const Context>& context) const
{
  return context->get_variable_value(this->name, loc).clone();
}

void Lookup::print(std::ostream& stream, const std::string&) const
{
  stream << this->name;
}
