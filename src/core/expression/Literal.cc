#include <string>
#include <memory>
#include <iostream>

#include <core/Context.h>

#include "Literal.h"

Value Literal::evaluate(const std::shared_ptr<const Context>&) const
{
  return value.clone();
}

void Literal::print(std::ostream& stream, const std::string&) const
{
  stream << value;
}
