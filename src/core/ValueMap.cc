
#include "ValueMap.h"
#include "Value.h"

  const Value& ValueMap::get(const std::string& name) const {
    auto result = map.find(name);
    return result == map.end() ? Value::undefined : result->second;
  }
