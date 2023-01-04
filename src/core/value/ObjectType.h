#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

class Value;
class EvaluationSession;
class str_utf8_wrapper;

// The object type which ObjectType's shared_ptr points to.
struct ObjectObject {
  using obj_t = std::unordered_map<std::string, Value>;
  obj_t map;
  class EvaluationSession *evaluation_session = nullptr;
  std::vector<std::string> keys;
  std::vector<Value> values;
};

class ObjectType{
protected:

    struct ObjectObjectDeleter {
      void operator()(ObjectObject *obj);
    };

private:
    explicit ObjectType(const std::shared_ptr<ObjectObject>& copy);
public:
    std::shared_ptr<ObjectObject> ptr;
    ObjectType(class EvaluationSession *session);
    ObjectType clone() const;
    const Value& get(const std::string& key) const;
    void set(const std::string& key, Value&& value);
    Value operator==(const ObjectType& v) const;
    Value operator<(const ObjectType& v) const;
    Value operator>(const ObjectType& v) const;
    Value operator!=(const ObjectType& v) const;
    Value operator<=(const ObjectType& v) const;
    Value operator>=(const ObjectType& v) const;
    const Value& operator[](const str_utf8_wrapper& v) const;
    const std::vector<std::string>& keys() const;
    friend std::ostream& operator<<(std::ostream& stream, const ObjectType& u);
  };

