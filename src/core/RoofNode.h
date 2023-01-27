// This file is a part of openscad. Everything implied is implied.
// Author: Alexey Korepanov <kaikaikai@yandex.ru>

#pragma once

#include <string>
#include <utility>

#include "AbstractNode.h"

class RoofNode final : public Visitable<AbstractPolyNode, RoofNode>{
public:
  RoofNode(NodeParams const & np) : Visitable{np} {}
  std::string toString() const override;
  std::string name() const override { return "roof"; }

  double fa, fs, fn;
  int convexity = 1;
  std::string method;

  class roof_exception : public std::exception
  {
public:
    roof_exception(std::string message) : m(std::move(message)) {}
    std::string message() {return m;}
private:
    std::string m;
  };
};
