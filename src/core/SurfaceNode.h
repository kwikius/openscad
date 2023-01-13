#pragma once

#include <vector>
#include <string>

#include "Filename.h"
#include "AbstractNode.h"

class Geometry;

typedef struct img_data_t
{
public:
  typedef double storage_type; // float could be enough here

  img_data_t() { min_val = 0; height = width = 0; }

  void clear(void) { min_val = 0; height = width = 0; storage.clear(); }

  void reserve(size_t x) { storage.reserve(x); }

  void resize(size_t x) { storage.resize(x); }

  storage_type& operator[](int x) { return storage[x]; }

  storage_type min_value() { return min_val; } // *std::min_element(storage.begin(), storage.end());

public:
  unsigned int height; // rows
  unsigned int width; // columns
  storage_type min_val;
  std::vector<storage_type> storage;

} img_data_t;

class SurfaceNode final : public Visitable<LeafNode, SurfaceNode>{
public:
  SurfaceNode(const ModuleInstantiation *mi) : Visitable(mi), center(false), invert(false), convexity(1) { }
  std::string toString() const override;
  std::string name() const override { return "surface"; }

  Filename filename;
  bool center;
  bool invert;
  int convexity;

  const Geometry *createGeometry() const override;
private:
  void convert_image(img_data_t& data, std::vector<uint8_t>& img, unsigned int width, unsigned int height) const;
  bool is_png(std::vector<uint8_t>& img) const;
  img_data_t read_dat(std::string filename) const;
  img_data_t read_png_or_dat(std::string filename) const;
};

