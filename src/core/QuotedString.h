#pragma once

#include <string>
#include <iosfwd>

class QuotedString : public std::string
{
public:
  QuotedString() : std::string() {}
  QuotedString(const std::string& s) : std::string(s) {}
};

std::ostream& operator<<(std::ostream& stream, const QuotedString& s);
