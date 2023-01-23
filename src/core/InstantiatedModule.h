#pragma once

#include "Location.h"

/**
*  @brief usefule bits of moduleInstantiation in AbstractModule
**/

class ModuleInstantiation;

class InstantiatedModule{
  Location loc;
  const void * const idx;
  bool tag_root;
  bool tag_highlight;
  bool tag_background;
  public:
  InstantiatedModule(ModuleInstantiation const & m);
  [[nodiscard]] bool isBackground() const { return this->tag_background; }
  [[nodiscard]] bool isHighlight() const { return this->tag_highlight; }
  [[nodiscard]] bool isRoot() const { return this->tag_root; }
  [[nodiscard]] const Location& getLocation() const { return this->loc; }
  const void * const getIdx() const { return idx;}
};
