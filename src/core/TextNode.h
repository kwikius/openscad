#pragma once

#include "AbstractNode.h"
#include "FreetypeRenderer.h"

class TextModule;

class TextNode final : public Visitable< AbstractPolyNode, TextNode>{
public:
 // VISITABLE();
  TextNode(const ModuleInstantiation *mi) : Visitable(mi) {}

  std::string toString() const override;
  std::string name() const override { return "text"; }

  virtual std::vector<const class Geometry *> createGeometryList() const;

  virtual FreetypeRenderer::Params get_params() const;

  FreetypeRenderer::Params params;
};
