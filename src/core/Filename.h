#pragma once

#include "QuotedString.h"

class Filename : public QuotedString
{
public:
  Filename() : QuotedString() {}
  Filename(const std::string& f) : QuotedString(f) {}
};
std::ostream& operator<<(std::ostream& stream, const Filename& filename);
