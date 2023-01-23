#pragma once

#include <memory>
#include <string>
#include <utility>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

class LocationImpl{
public:
  LocationImpl(int firstLine, int firstCol, int lastLine, int lastCol,
           std::shared_ptr<fs::path> path)
    : first_line(firstLine), first_col(firstCol), last_line(lastLine),
    last_col(lastCol), path(std::move(path)) {
  }

  std::string fileName() const { return path ? path->generic_string() : ""; }
  const fs::path& filePath() const { return *path; }
  int firstLine() const { return first_line; }
  int firstColumn() const { return first_col; }
  int lastLine() const { return last_line; }
  int lastColumn() const { return last_col; }
private:
  int first_line;
  int first_col;
  int last_line;
  int last_col;
  std::shared_ptr<fs::path> path;
};

class Location {
public:
  static const Location NONE;
   Location(int firstLine, int firstCol, int lastLine, int lastCol,
           std::shared_ptr<fs::path> path)
    : impl{std::make_shared<LocationImpl>(firstLine,firstCol,lastLine,lastCol,path)}
   {}
  bool operator==(Location const& rhs)const;
  bool operator!=(Location const& rhs)const;
  bool isNone() const;

  std::string fileName() const { return this->impl->fileName(); }
  const fs::path& filePath() const { return this->impl->filePath(); }
  int firstLine() const { return this->impl->firstLine(); }
  int firstColumn() const { return this->impl->firstColumn(); }
  int lastLine() const { return this->impl->lastLine(); }
  int lastColumn() const { return this->impl->lastColumn(); }

  std::string toRelativeString(const std::string& docPath) const;

  private:
  std::shared_ptr<LocationImpl> impl;
};
