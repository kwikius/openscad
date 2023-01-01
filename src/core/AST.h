#pragma once

#include <string>
#include <memory>
#include <iosfwd>
#include "Location.h"

class ASTNode
{
public:

  ASTNode(Location loc) : loc(std::move(loc)) {}
  virtual ~ASTNode() = default;
  virtual void print(std::ostream& stream, const std::string& indent) const = 0;
  [[nodiscard]] std::string dump(const std::string& indent) const;
  [[nodiscard]] const Location& location() const { return loc; }

  void setLocation(const Location& loc) { this->loc = loc; }

protected:
  Location loc;
};

std::ostream& operator<<(std::ostream& stream, const ASTNode& ast);
