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

// The base visitor class see NodeVisitor.h
class NodeVisitor;

class BaseVisitable{
public:
  virtual ~BaseVisitable() = default;
  [[nodiscard]] virtual Response accept(State&, NodeVisitor&) const = 0;
};

// B is derived from BaseVisitable. Here it is always an AbstractNode
// and in fact we could just put the pure virtual in AbstractNode in this case
// D is derived from Visitable<B,D>
template <typename B , typename D>
class Visitable : public B {
protected:
   // Forward all arguments to base class from derived class
   template <typename... Args>
   Visitable(Args&&... args) : B{std::forward<Args>(args)...}{}
public:
   [[nodiscard]] Response accept(State &state, NodeVisitor &visitor) const override;
};
