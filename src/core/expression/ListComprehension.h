#pragma once

#include "../Expression.h"

class ListComprehension : public Expression
{
public:
  ListComprehension(const Location& loc);
  ~ListComprehension() = default;
};
