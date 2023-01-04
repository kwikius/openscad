
#include <sstream>
#include <core/Value.h>

Value UndefType::operator<(const UndefType& other) const {
  return Value::undef("operation undefined (undefined < undefined)");
}
Value UndefType::operator>(const UndefType& other) const {
  return Value::undef("operation undefined (undefined > undefined)");
}
Value UndefType::operator<=(const UndefType& other) const {
  return Value::undef("operation undefined (undefined <= undefined)");
}
Value UndefType::operator>=(const UndefType& other) const {
  return Value::undef("operation undefined (undefined >= undefined)");
}

std::string UndefType::toString() const {
  std::ostringstream stream;
  if (!reasons->empty()) {
    auto it = reasons->begin();
    stream << *it;
    for (++it; it != reasons->end(); ++it) {
      stream << "\n\t" << *it;
    }
  }
  // clear reasons so multiple same warnings are not given on the same value
  reasons->clear();
  return stream.str();
}

std::ostream& operator<<(std::ostream& stream, const UndefType& u)
{
  stream << "undef";
  return stream;
}
