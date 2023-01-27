#pragma once

#include <boost/optional.hpp>

#include "AbstractNode.h"
#include "Filename.h"

enum class ImportType {
  UNKNOWN,
  AMF,
  _3MF,
  STL,
  OFF,
  SVG,
  DXF,
  NEF3,
};

class ImportNode final : public Visitable<LeafNode, ImportNode>{
public:
  constexpr static double SVG_DEFAULT_DPI = 72.0;
  ImportNode(NodeParams const & np, ImportType type)
  : Visitable{np}, type(type) { }
  std::string toString() const override;
  std::string name() const override;

  ImportType type;
  Filename filename;
  boost::optional<std::string> id;
  boost::optional<std::string> layer;
  int convexity;
  bool center;
  double dpi;
  double fn, fs, fa;
  double origin_x, origin_y, scale;
  double width, height;
  const class Geometry *createGeometry() const override;
};
