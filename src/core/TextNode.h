#pragma once

#include "AbstractNode.h"
#include "FreetypeRenderer.h"

class TextModule;

class TextNode final : public Visitable< AbstractPolyNode, TextNode>{
public:
 // VISITABLE();
  TextNode(NodeParams const & np) : Visitable{np} {}

  std::string toString() const override;
  std::string name() const override { return "text"; }

  virtual std::vector<const class Geometry *> createGeometryList() const;

  virtual FreetypeRenderer::Params get_params() const;

  FreetypeRenderer::Params params;
};
