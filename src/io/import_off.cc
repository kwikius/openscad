

#include <utils/printutils.h>

#include <geometry/PolySet.h>

#include <core/Location.h>

#ifdef ENABLE_CGAL
#include <geometry/cgal/cgalutils.h>
#endif
#include "import.h"

PolySet *import_off(const std::string& filename, const Location& loc)
{
  PolySet *p = new PolySet(3);
#ifdef ENABLE_CGAL
  CGAL_Polyhedron poly;
  std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary);
  if (!file.good()) {
    LOG(message_group::Warning, Location::NONE, "", "Can't open import file '%1$s', import() at line %2$d", filename, loc.firstLine());
  } else {
    file >> poly;
    file.close();
    CGALUtils::createPolySetFromPolyhedron(poly, *p);
  }
#else
  LOG(message_group::Warning, Location::NONE, "", "OFF import requires CGAL, import() at line %2$d", filename, loc.firstLine());
#endif // ifdef ENABLE_CGAL
  return p;
}

