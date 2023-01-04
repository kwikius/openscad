
#include "ObjectType.h"

#include <core/Value.h>
#include "str_utf8_wrapper.h"

// called by clone()
ObjectType::ObjectType(const std::shared_ptr<ObjectObject>& copy)
  : ptr(copy)
{
}

ObjectType::ObjectType(EvaluationSession *session) :
  ptr(std::shared_ptr<ObjectObject>(new ObjectObject()))
{
  ptr->evaluation_session = session;
}

const Value& ObjectType::get(const std::string& key) const
{
  auto result = ptr->map.find(key);
  // NEEDSWORK it would be nice to have a "cause" for the undef, but Value::undef(...)
  // does not appear compatible with Value&.
  return result == ptr->map.end() ? Value::undefined : result->second;
}

void ObjectType::set(const std::string& key, Value&& value)
{
  ptr->map.emplace(key, std::move(value));
  ptr->keys.emplace_back(key);
  ptr->values.emplace_back(std::move(value));
}

const std::vector<std::string>& ObjectType::keys() const
{
  return ptr->keys;
}

const Value& ObjectType::operator[](const str_utf8_wrapper& v) const
{
  return this->get(v.toString());
}

// Copy explicitly only when necessary
ObjectType ObjectType::clone() const
{
  return ObjectType(this->ptr);
}

std::ostream& operator<<(std::ostream& stream, const ObjectType& v)
{
  stream << "{ ";
  auto iter = v.ptr->keys.begin();
  if (iter != v.ptr->keys.end()) {
    str_utf8_wrapper k(*iter);
    for (; iter != v.ptr->keys.end(); ++iter) {
      str_utf8_wrapper k2(*iter);
      stream << k2.toString() << " = " << v[k2] << "; ";
    }
  }
  stream << "}";
  return stream;
}

Value ObjectType::operator==(const ObjectType& other) const {
  return Value::undef("operation undefined (object == object)");
}
Value ObjectType::operator!=(const ObjectType& other) const {
  return Value::undef("operation undefined (object != object)");
}
Value ObjectType::operator<(const ObjectType& other) const {
  return Value::undef("operation undefined (object < object)");
}
Value ObjectType::operator>(const ObjectType& other) const {
  return Value::undef("operation undefined (object > object)");
}
Value ObjectType::operator<=(const ObjectType& other) const {
  return Value::undef("operation undefined (object <= object)");
}
Value ObjectType::operator>=(const ObjectType& other) const {
  return Value::undef("operation undefined (object >= object)");
}
