#pragma once

#include "Response.h"
#include "State.h"

template <typename VisitableType>
class Visitor{
public:
  [[nodiscard]] virtual Response visit(State& state, const VisitableType&) = 0;
};

template< typename A>
struct BaseVisitor : Visitor<A>{
  virtual ~BaseVisitor() = default;
};

template <typename VisitableType, typename VisitorList>
struct DerivedVisitor : Visitor<VisitableType>{
   [[nodiscard]] Response visit(State & state, VisitableType const & node) override
   {return static_cast<VisitorList&>(*this).visitBaseClass(state,node);}
};

template <typename A, typename ...T>
struct VisitorList;

template <typename Visitable>
using getVisitableBaseType = typename Visitable::visitableBaseType;

template <typename A, typename ...T>
struct VisitorList : BaseVisitor<A>, DerivedVisitor<T, VisitorList<A,T...> >... {
  using Visitor<T>::visit ...;
  using Visitor<A>::visit;
  template <typename V>
    requires (!std::is_same_v<V,A>)
  [[nodiscard]] Response visitBaseClass(State& state,V const & v)
  {
    return this->visit(state, static_cast< getVisitableBaseType<V> const & >(v));
  }
};

struct NodeVisitor : VisitorList<
   class AbstractNode,
   class AbstractIntersectionNode,
   class AbstractPolyNode,
   class ListNode,
   class GroupNode,
   class RootNode,
   class LeafNode,
   class CgalAdvNode,
   class CsgOpNode,
   class LinearExtrudeNode,
   class RotateExtrudeNode,
   class RoofNode,
   class ImportNode,
   class TextNode,
   class ProjectionNode,
   class RenderNode,
   class SurfaceNode,
   class TransformNode,
   class ColorNode,
   class OffsetNode
  >{
    NodeVisitor() = default;
    Response traverse(const AbstractNode& node, const State& state = NodeVisitor::nullstate);
    private:
    static const State nullstate;
};
