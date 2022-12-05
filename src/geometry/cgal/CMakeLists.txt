#cgal lib

set(CGAL_LIB_SOURCES
  ../GeometryEvaluator.cc
  cgalutils.cc
  cgalutils-applyops.cc
  cgalutils-applyops-hybrid.cc
  cgalutils-applyops-hybrid-minkowski.cc
  cgalutils-closed.cc
  cgalutils-convex.cc
  cgalutils-corefine.cc
  cgalutils-kernel.cc
  cgalutils-hybrid.cc
  cgalutils-mesh.cc
  cgalutils-minkowski.cc
  cgalutils-nef.cc
  cgalutils-orient.cc
  cgalutils-polyhedron.cc
  cgalutils-project.cc
  cgalutils-tess.cc
  cgalutils-triangulate.cc
  CGALHybridPolyhedron.cc
  CGAL_Nef_polyhedron.cc
  CGALCache.cc
  Polygon2d-CGAL.cc
  ../../io/export_nef.cc
  ../../io/import_nef.cc
)

add_library(openscadCGAL ${CGAL_LIB_SOURCES})

