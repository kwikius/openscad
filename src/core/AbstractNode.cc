/*
 *  OpenSCAD (www.openscad.org)
 *  Copyright (C) 2009-2011 Clifford Wolf <clifford@clifford.at> and
 *                          Marius Kintel <marius@kintel.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  As a special exception, you have permission to link this program
 *  with the CGAL library and distribute executables, as long as you
 *  follow the requirements of the GNU GPL in regard to all of the
 *  software in the executable aside from CGAL.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <functional>
#include <iostream>
#include <algorithm>

#include "AbstractNode.h"
#include "ModuleInstantiation.h"
#include "progress.h"
#include "RootNode.h"

#include "Visitable_inline.h"

InstantiatedModule::InstantiatedModule(ModuleInstantiation const & mi)
 : loc{mi.location()}
  ,idx{static_cast<const void* const>(&mi)}
  ,tag_root{mi.isRoot()}
  ,tag_highlight{mi.isHighlight()}
  ,tag_background{mi.isBackground()}
{}

InstantiatedModule::InstantiatedModule(Location const & loc)
 : loc{loc}
  ,idx{nullptr}
  ,tag_root{false}
  ,tag_highlight{false}
  ,tag_background{false}
{}

size_t AbstractNode::idx_counter;

AbstractNode::AbstractNode(const ModuleInstantiation *mi) :
  modinst(*mi),
  idx(idx_counter++)
{
}

AbstractNode::AbstractNode(InstantiatedModule const & im) :
  modinst(im),
  idx(idx_counter++)
{
}

std::string AbstractNode::toString() const
{
  return this->name() + "()";
}

[[nodiscard]] Location const & AbstractNode::getLocation() const { return modinst.getLocation();}
[[nodiscard]] bool AbstractNode::isRoot() const {return modinst.isRoot();}
[[nodiscard]] bool AbstractNode::isBackground() const { return modinst.isBackground();}
[[nodiscard]] bool AbstractNode::isHighlight() const { return modinst.isHighlight();}

[[nodiscard]] bool AbstractNode::hasSameModInst(std::shared_ptr<AbstractNode> const & other) const
{ return this->modinst.getIdx() == other->modinst.getIdx();}

std::shared_ptr<const AbstractNode>
AbstractNode::getNodeByID(int idx, std::deque<std::shared_ptr<const AbstractNode>>& path) const
{
  auto self = shared_from_this();
  if (this->idx == idx) {
    path.push_back(self);
    return self;
  }
  for (const auto& node : this->children) {
    auto res = node->getNodeByID(idx, path);
    if (res) {
      path.push_back(self);
      return res;
    }
  }
  return nullptr;
}

std::string GroupNode::name() const
{
  return "group";
}

std::string GroupNode::verbose_name() const
{
  return this->_name;
}

std::string ListNode::name() const
{
  return "list";
}

std::string RootNode::name() const
{
  return "root";
}

std::string AbstractIntersectionNode::toString() const
{
  return this->name() + "()";
}

std::string AbstractIntersectionNode::name() const
{
  // We write intersection here since the module will have to be evaluated
  // before we get here and it will not longer retain the intersection_for parameters
  return "intersection";
}

void AbstractNode::progress_prepare()
{
  std::for_each(this->children.begin(), this->children.end(), std::mem_fn(&AbstractNode::progress_prepare));
  this->progress_mark = ++progress_report_count;
}

void AbstractNode::progress_report() const
{
  progress_update(shared_from_this(), this->progress_mark);
}

std::ostream& operator<<(std::ostream& stream, const AbstractNode& node)
{
  stream << node.toString();
  return stream;
}

/*!
   Locates and returns the node containing a root modifier (!).
   Returns nullptr if no root modifier was found.
   If a second root modifier was found, nextLocation (if non-zero) will be set to point to
   the location of that second modifier.
 */
std::shared_ptr<const AbstractNode> find_root_tag(const std::shared_ptr<const AbstractNode>& node, const Location **nextLocation)
{
  std::shared_ptr<const AbstractNode> rootTag;

  std::function<void (const std::shared_ptr<const AbstractNode>&)> recursive_find_tag
   = [&](const std::shared_ptr<const AbstractNode>& node) {
      for (const auto& child : node->getChildren()) {
        if (child->isRoot()) {
          if (!rootTag) {
            rootTag = child;
            // shortcut if we're not interested in further root modifiers
            if (!nextLocation) return;
          } else if (nextLocation && (!rootTag->hasSameModInst(child)) ) { //(&rootTag->getModInst() != &child->getModInst()) ) {
            // Throw if we have more than one root modifier in the source
            *nextLocation = &child->getLocation();
            return;
          }
        }
        recursive_find_tag(child);
      }
    };

  recursive_find_tag(node);

  return rootTag;
}
