#pragma once

#include <iosfwd>

#include <core/Expression.h>

class Lookup final : public Expression
{
public:
  Lookup(const std::string& name, const Location& loc);
  Value evaluate(const std::shared_ptr<const Context>& context) const override;
  void print(std::ostream& stream, const std::string& indent) const override;
  const std::string& get_name() const { return name; }
private:
  std::string name;
};
