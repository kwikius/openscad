#pragma once

#include <iostream>
#include <type_traits>

enum class Response {ContinueTraversal, AbortTraversal, PruneTraversal};

class State;
class NodeVisitor;

template <typename VisitableType>
class Visitor{
public:
  [[nodiscard]] virtual Response visit(State& state, const VisitableType&) = 0;
};

template <typename B , typename D>
class Visitable : public B {
public:
   using visitableBaseType = B;
public:
   template <typename... Args>
   Visitable(Args&&... args) : B{std::forward<Args>(args)...}{}
public:
   [[nodiscard]] Response accept(State &state, NodeVisitor &visitor) const override;
};

template <typename Root>
class Visitable<void,Root>{
public:
  virtual ~Visitable() = default;
  [[nodiscard]] virtual Response accept(State& state , NodeVisitor& visitor) const = 0;
};
