#include "QuotedString.h"

#include <iostream>

// FIXME: This could probably be done more elegantly using boost::regex
std::ostream& operator<<(std::ostream& stream, const QuotedString& s)
{
  stream << '"';
  for (char c : s) {
    switch (c) {
    case '\t': stream << "\\t"; break;
    case '\n': stream << "\\n"; break;
    case '\r': stream << "\\r"; break;
    case '"':  stream << "\\\""; break;
    case '\\': stream << "\\\\"; break;
    default:   stream << c;
    }
  }
  return stream << '"';
}
