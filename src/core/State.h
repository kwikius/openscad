#pragma once

#include <cstring>
#include <memory>

#include <geometry/linalg.h>

#include "AbstractNode.h"

/**
  * &brief struct for returning various AbstractNode state
  * usually from a Visitor
**/
class State{
public:
  State(const std::shared_ptr<const AbstractNode> &parent)
    : flags(NONE), parentnode(parent), numchildren(0) {
    this->matrix_ = Transform3d::Identity();
    this->color_.fill(-1.0f);
  }
  virtual ~State() {}

private:
    enum StateFlags {
    NONE       = 0x00,
    PREFIX     = 0x01,
    POSTFIX    = 0x02,
    PREFERNEF  = 0x04,
    HIGHLIGHT  = 0x08,
    BACKGROUND = 0x10
   };
   template <StateFlags Flag>
   void setFlag( bool v) { this->flags = (v)?(this->flags | Flag ):(this->flags & ~Flag);}
   template <StateFlags Flag>
   bool getFlag() const { return (this->flags & Flag) ? true : false;}
public:
  void setPrefix(bool v) { setFlag<PREFIX>(v); }
  bool isPrefix() const { return getFlag<PREFIX>(); }
  void setPostfix(bool v) { setFlag<POSTFIX>(v); }
  bool isPostfix() const { return getFlag<POSTFIX>(); }
  void setHighlight(bool v) { setFlag<HIGHLIGHT>(v); }
  bool isHighlight() const { return getFlag<HIGHLIGHT>(); }
  void setBackground(bool v) { setFlag<BACKGROUND>(v); }
  bool isBackground() const { return getFlag<BACKGROUND>(); }
  void setPreferNef(bool v) { setFlag<PREFERNEF>(v); }
  bool preferNef() const { return getFlag<PREFERNEF>(); }

  void setNumChildren(unsigned int numc) { this->numchildren = numc; }
  void setParent(const std::shared_ptr<const AbstractNode> &parent) { this->parentnode = parent; }
  void setMatrix(const Transform3d& m) { this->matrix_ = m; }
  void setColor(const Color4f& c) { this->color_ = c; }

  unsigned int numChildren() const { return this->numchildren; }
  std::shared_ptr<const AbstractNode> parent() const { return this->parentnode; }
  const Transform3d& matrix() const { return this->matrix_; }
  const Color4f& color() const { return this->color_; }

private:

  unsigned int flags;
  std::shared_ptr<const AbstractNode> parentnode;
  unsigned int numchildren;

  // Transformation matrix and color. FIXME: Generalize such state variables?
  Transform3d matrix_;
  Color4f color_;
};
