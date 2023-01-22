
#include "NodeVisitor.h"
#include "Visitables.h"
#include "State.h"

State NodeVisitor::nullstate(nullptr);

Response NodeVisitor::traverse(const AbstractNode& node, const State& state)
{
  State newstate = state;
  newstate.setNumChildren(node.getChildren().size());
  newstate.setPrefix(true);
  newstate.setParent(state.parent());
  auto response = node.accept(newstate, *this);

  // Pruned traversals mean don't traverse children
  if (response == Response::ContinueTraversal) {
    newstate.setParent(node.shared_from_this());
    for (const auto& chnode : node.getChildren()) {
      response = this->traverse(*chnode, newstate);
      if (response == Response::AbortTraversal) return response; // Abort immediately
    }
  }

  // Postfix is executed for all non-aborted traversals
  if (response != Response::AbortTraversal) {
    newstate.setParent(state.parent());
    newstate.setPrefix(false);
    newstate.setPostfix(true);
    response = node.accept(newstate, *this);
  }

  return (response == Response::AbortTraversal)
     ?  Response::AbortTraversal
     :  Response::ContinueTraversal
  ;
}
