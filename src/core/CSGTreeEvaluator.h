#pragma once

#include <map>
#include <list>
#include <vector>
#include <cstddef>

#include "NodeVisitor.h"
#include "CSGNode.h"

class CSGTreeEvaluator : public NodeVisitor
{
public:
  CSGTreeEvaluator(const class Tree& tree, class GeometryEvaluator *geomevaluator = nullptr)
    : tree(tree), geomevaluator(geomevaluator) {
  }
  ~CSGTreeEvaluator() {}

  Response visit(State& state, const class AbstractNode& node) override;
  Response visit(State& state, const class AbstractIntersectionNode& node) override;
  Response visit(State& state, const class AbstractPolyNode& node) override;
  Response visit(State& state, const class ListNode& node) override;
  Response visit(State& state, const class CsgOpNode& node) override;
  Response visit(State& state, const class TransformNode& node) override;
  Response visit(State& state, const class ColorNode& node) override;
  Response visit(State& state, const class RenderNode& node) override;
  Response visit(State& state, const class CgalAdvNode& node) override;

  std::shared_ptr<class CSGNode> buildCSGTree(const AbstractNode& node);

  const std::shared_ptr<CSGNode>& getRootNode() const {
    return this->rootNode;
  }
  const std::vector<std::shared_ptr<CSGNode>>& getHighlightNodes() const {
    return this->highlightNodes;
  }
  const std::vector<std::shared_ptr<CSGNode>>& getBackgroundNodes() const {
    return this->backgroundNodes;
  }

private:
  void addToParent(const State& state, const AbstractNode& node);
  void applyToChildren(State& state, const AbstractNode& node, OpenSCADOperator op);
  std::shared_ptr<CSGNode> evaluateCSGNodeFromGeometry(State& state,
                                                  const std::shared_ptr<const class Geometry>& geom,
                                                  const class ModuleInstantiation *modinst,
                                                    const AbstractNode& node);
  void applyBackgroundAndHighlight(State& state, const AbstractNode& node);

  typedef std::list<std::shared_ptr<const AbstractNode>> ChildList;
  std::map<int, ChildList> visitedchildren;

protected:
  const Tree& tree;
  class GeometryEvaluator *geomevaluator;
  std::shared_ptr<CSGNode> rootNode;
  std::vector<std::shared_ptr<CSGNode>> highlightNodes;
  std::vector<std::shared_ptr<CSGNode>> backgroundNodes;
  std::map<int, std::shared_ptr<CSGNode>> stored_term; // The term evaluated from each node index
};
