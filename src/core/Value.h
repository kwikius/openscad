#pragma once
/*
Copyright (C) Andy Little (kwikius@yahoo.com) 10/10/2022  add module_reference value type
https://github.com/openscad/openscad/blob/master/COPYING
*/

#include <vector>
#include <string>
//#include <algorithm>
#include <cstdint>
#include <limits>
#include <iostream>
#include <memory>
#include <variant>

#include <glib.h>

#include "Assignment.h"

#include <core/value/VectorType.h>
#include <core/value/EmbeddedVectorType.h>
#include <core/value/ValuePtr.h>
#include <core/value/ModuleReference.h>
#include <core/value/RangeType.h>
#include <core/value/FunctionType.h>
#include <core/value/ObjectType.h>
#include <core/value/UndefType.h>
#include <core/value/str_utf8_wrapper.h>

class QuotedString;
class VectorType;
template <typename T> class ValuePtr;
class ModuleReference;
class RangeType;
class FunctionType;
class ObjectType;
class str_utf8_wrapper;

class tostring_visitor;
class tostream_visitor;
class Context;
class Expression;
class Value;

using FunctionPtr = ValuePtr<FunctionType>;
using ModuleReferencePtr = ValuePtr<ModuleReference>;
using RangePtr = ValuePtr<RangeType>;

/**
 *  @brief Value class encapsulates a std::variant value which can represent any of the
 *  value types existing in the SCAD language.
 * -- As part of a refactoring effort which began as PR #2881 and continued as PR #3102,
 *    Value and its constituent types have been made (nominally) "move only".
 * -- In some cases a copy of a Value is necessary or unavoidable, in which case Value::clone() can be used.
 * -- Value::clone() is used instead of automatic copy construction/assignment so this action is
 *    made deliberate and explicit (and discouraged).
 * -- Recommended to make use of RVO (Return Value Optimization) wherever possible:
 *       https://en.cppreference.com/w/cpp/language/copy_elision
 * -- Classes which cache Values such as Context or dxf_dim_cache(see dxfdim.cc), when queried
 *    should return either a const reference or a clone of the cached value if returning by-value.
 *    NEVER return a non-const reference!
 */
class Value{
public:
  enum class Type {
    UNDEFINED,
    BOOL,
    NUMBER,
    STRING,
    VECTOR,
    EMBEDDED_VECTOR,
    RANGE,
    FUNCTION,
    OBJECT,
    MODULE
  };
  // FIXME: eventually remove this in favor of specific messages for each undef usage
  static const Value undefined;

private:
  Value() : value(UndefType()) { } // Don't default construct empty Values.  If "undefined" needed, use reference to Value::undefined, or call Value::undef() for return by value
public:
  Value(const Value&) = delete; // never copy, move instead
  Value& operator=(const Value& v) = delete; // never copy, move instead
  Value(Value&&) = default;
  Value& operator=(Value&&) = default;
  Value clone() const; // Use sparingly to explicitly copy a Value

  Value(int v) : value(double(v)) { }
  Value(const char *v) : value(str_utf8_wrapper(v)) { } // prevent insane implicit conversion to bool!
  Value(char *v) : value(str_utf8_wrapper(v)) { } // prevent insane implicit conversion to bool!
                                                  // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0608r3.html
  template <class T> Value(T&& val) : value(std::forward<T>(val)) { }

  static Value undef(const std::string& why); // creation of undef requires a reason!

  const std::string typeName() const;
  static std::string typeName(Type type);
  Type type() const { return static_cast<Type>(this->value.index()); }
  bool isDefinedAs(const Type type) const { return this->type() == type; }
  bool isDefined()   const { return this->type() != Type::UNDEFINED; }
  bool isUndefined() const { return this->type() == Type::UNDEFINED; }
  bool isUncheckedUndef() const;

  // Conversion to std::variant "BoundedType"s. const ref where appropriate.
  bool toBool() const;
  double toDouble() const;
  const str_utf8_wrapper& toStrUtf8Wrapper() const;
  const VectorType& toVector() const;
  const EmbeddedVectorType& toEmbeddedVector() const;
  VectorType& toVectorNonConst();
  EmbeddedVectorType& toEmbeddedVectorNonConst();
  const RangeType& toRange() const;
  const FunctionType& toFunction() const;
  const ObjectType& toObject() const;
  const ModuleReference& toModuleReference() const;

  // Other conversion utility functions
  bool getDouble(double& v) const;
  bool getFiniteDouble(double& v) const;
  bool getUnsignedInt(unsigned int& v) const;
  bool getPositiveInt(unsigned int& v) const;
  std::string toString() const;
  std::string toEchoString() const;
  std::string toEchoStringNoThrow() const; //use this for warnings
  const UndefType& toUndef();
  std::string toUndefString() const;
  std::string chrString() const;
  bool getVec2(double& x, double& y, bool ignoreInfinite = false) const;
  bool getVec3(double& x, double& y, double& z) const;
  bool getVec3(double& x, double& y, double& z, double defaultval) const;

  // Common Operators
  operator bool() const = delete;
  Value operator==(const Value& v) const;
  Value operator!=(const Value& v) const;
  Value operator<(const Value& v) const;
  Value operator<=(const Value& v) const;
  Value operator>=(const Value& v) const;
  Value operator>(const Value& v) const;
  Value operator-() const;
  Value operator[](size_t idx) const;
  Value operator[](const Value& v) const;
  Value operator+(const Value& v) const;
  Value operator-(const Value& v) const;
  Value operator*(const Value& v) const;
  Value operator/(const Value& v) const;
  Value operator%(const Value& v) const;
  Value operator^(const Value& v) const;

  static bool cmp_less(const Value& v1, const Value& v2);

  friend std::ostream& operator<<(std::ostream& stream, const Value& value);

  typedef std::variant<UndefType, bool, double, str_utf8_wrapper,
    VectorType, EmbeddedVectorType, RangePtr, FunctionPtr, ObjectType,
    ModuleReferencePtr
   > Variant;

  static_assert(sizeof(Value::Variant) <= 24, "Memory size of Value too big");
  const Variant& getVariant() const { return value; }
  Variant & getVariant() { return value;}
private:
  Variant value;
};

