#pragma once

#include <string>
#include <memory>
#include <iosfwd>

#include "Location.h"

class ASTNode
{
public:
  ASTNode(const Location& loc) : loc(loc) {}
  virtual ~ASTNode() {}
  virtual void print(std::ostream& stream, const std::string& indent) const = 0;
  std::string dump(const std::string& indent) const;
  const Location& location() const { return loc; }
  void setLocation(const Location& loc) { this->loc = loc; }

protected:
  Location loc;
};

std::ostream& operator<<(std::ostream& stream, const ASTNode& ast);
