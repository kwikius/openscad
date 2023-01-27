#pragma once

#include "Location.h"

class NodeParams{
  Location loc;
  const void * const idx;
  bool tag_root;
  bool tag_highlight;
  bool tag_background;
  public:

  NodeParams(Location const & loc, const void* const idx,
    bool root,bool highlight, bool background);

  NodeParams(Location const & loc);
  [[nodiscard]] bool isBackground() const { return this->tag_background; }
  [[nodiscard]] bool isHighlight() const { return this->tag_highlight; }
  [[nodiscard]] bool isRoot() const { return this->tag_root; }
  [[nodiscard]] const Location& getLocation() const { return this->loc; }
  const void * const getIdx() const { return idx;}
};
