#pragma once

#include <glview/VBORenderer.h>
#include <glview/cgal/CGAL_OGL_Polyhedron.h>
#include <geometry/cgal/CGAL_Nef_polyhedron.h>

class CGALRenderer : public VBORenderer
{
public:
  CGALRenderer(std::shared_ptr<const class Geometry> geom);
  ~CGALRenderer();
  void prepare(bool showfaces, bool showedges, const shaderinfo_t *shaderinfo = nullptr) override;
  void draw(bool showfaces, bool showedges, const shaderinfo_t *shaderinfo = nullptr) const override;
  void setColorScheme(const ColorScheme& cs) override;
  BoundingBox getBoundingBox() const override;

private:
  void addGeometry(const std::shared_ptr<const class Geometry>& geom);
  const std::list<std::shared_ptr<class CGAL_OGL_Polyhedron>>& getPolyhedrons() const { return this->polyhedrons; }
  void createPolyhedrons();
  void createPolySets();
  bool last_render_state; // FIXME: this is temporary to make switching between renderers seamless.

  std::list<std::shared_ptr<class CGAL_OGL_Polyhedron>> polyhedrons;
  std::list<std::shared_ptr<const class PolySet>> polysets;
  std::list<std::shared_ptr<const CGAL_Nef_polyhedron>> nefPolyhedrons;

  VertexStates polyset_states;
  GLuint polyset_vertices_vbo;
  GLuint polyset_elements_vbo;
};
