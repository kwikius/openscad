
#include <utils/boost-utils.h>
#include <iostream>
#include "Filename.h"

namespace fs = boost::filesystem;

std::ostream& operator<<(std::ostream& stream, const Filename& filename)
{
  fs::path fnpath{static_cast<std::string>(filename)}; // gcc-4.6
  auto fpath = boostfs_uncomplete(fnpath, fs::current_path());
  stream << QuotedString(fpath.generic_string());
  return stream;
}
