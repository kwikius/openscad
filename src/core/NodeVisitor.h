#pragma once

#include "BaseVisitable.h"
#include "State.h"
#include "AbstractNode.h"
#include "CgalAdvNode.h"
#include "CsgOpNode.h"
#include "TransformNode.h"
#include "TextNode.h"
#include "ColorNode.h"
#include "LinearExtrudeNode.h"
#include "RotateExtrudeNode.h"
#include "OffsetNode.h"
#include "RoofNode.h"
#include "ImportNode.h"
#include "RenderNode.h"
#include "ProjectionNode.h"
#include "SurfaceNode.h"
#include "RootNode.h"

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
   AbstractNode,
   AbstractIntersectionNode,
   AbstractPolyNode,
   ListNode,
   GroupNode,
   RootNode,
   LeafNode,
   CgalAdvNode,
   CsgOpNode,
   LinearExtrudeNode,
   RotateExtrudeNode,
   RoofNode,
   ImportNode,
   TextNode,
   ProjectionNode,
   RenderNode,
   SurfaceNode,
   TransformNode,
   ColorNode,
   OffsetNode
  >{
    NodeVisitor() = default;
    Response traverse(const AbstractNode& node, const State& state = NodeVisitor::nullstate);
    private:
    static State nullstate;
};

template <typename B , typename D>
[[nodiscard]]  inline Response
Visitable<B,D>::accept(State &state, NodeVisitor &visitor) const
{
   return static_cast<Visitor<D>&>(visitor).visit(
      state, static_cast<D const &>(*this)
   );
}
