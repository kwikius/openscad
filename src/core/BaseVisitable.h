#pragma once

#include <iostream>
#include <type_traits>

// FIXME: Default constructor Response()
enum class Response {ContinueTraversal, AbortTraversal, PruneTraversal};

class State;

class BaseVisitor{
public:
  virtual ~BaseVisitor() = default;
};

template <class T>
class Visitor{
public:
  [[nodiscard]] virtual Response visit(State& state, const T&) = 0;
};

class NodeVisitor;

class BaseVisitable{
public:
  virtual ~BaseVisitable() = default;
  [[nodiscard]] virtual Response accept(State&, NodeVisitor&) const = 0;
};

// B is derived from NodeVisitor
// D is derived from Visitable<D>
template <typename B , typename D>
class Visitable : public B {
public:
   template <typename... Args>
   Visitable(Args&&... args) : B{std::forward<Args>(args)...}{}
   [[nodiscard]] Response accept(State &state, NodeVisitor &visitor) const override;
};
