
#include <sstream>

#include <utils/boost-utils.h>

#include "AST.h"

const Location Location::NONE(0, 0, 0, 0, std::make_shared<fs::path>(fs::path{}));

bool Location::operator==(Location const& rhs)const
{
  return
    this->firstLine() == rhs.firstLine() &&
    this->firstColumn() == rhs.firstColumn() &&
    this->lastLine() == rhs.lastLine() &&
    this->lastColumn() == rhs.lastColumn() &&
    this->filePath() == rhs.filePath();
}

bool Location::operator!=(Location const& rhs)const
{
  return !(*this == rhs);
}

bool Location::isNone() const {
  return (*this == Location::NONE);
}

std::string Location::toRelativeString(const std::string& docPath) const {
  if (this->isNone()) return "location unknown";
  return "in file " + boostfs_uncomplete((*path), docPath).generic_string() + ", " + "line " + std::to_string(this->firstLine());
}

std::ostream& operator<<(std::ostream& stream, const ASTNode& ast)
{
  ast.print(stream, "");
  return stream;
}

std::string ASTNode::dump(const std::string& indent) const
{
  std::ostringstream stream;
  print(stream, indent);
  return stream.str();
}
