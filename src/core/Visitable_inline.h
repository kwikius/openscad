#pragma once

#include "NodeVisitor.h"
#include "Visitable.h"

template <typename B , typename D>
[[nodiscard]]  inline Response
Visitable<B,D>::accept(State &state, NodeVisitor &visitor) const
{
   return static_cast<Visitor<D>&>(visitor).visit(
      state, static_cast<D const &>(*this)
   );
}
