/*
 *  OpenSCAD (www.openscad.org)
 *  Copyright (C) 2009-2011 Clifford Wolf <clifford@clifford.at> and
 *                          Marius Kintel <marius@kintel.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  As a special exception, you have permission to link this program
 *  with the CGAL library and distribute executables, as long as you
 *  follow the requirements of the GNU GPL in regard to all of the
 *  software in the executable aside from CGAL.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/*
Copyright (C) Andy Little (kwikius@yahoo.com) 10/10/2022  add module_reference value type
https://github.com/openscad/openscad/blob/master/COPYING
*/

#include <cassert>
#include <cmath>
#include <numeric>
#include <sstream>
/*Unicode support for string lengths and array accesses*/
#include <glib.h>
#include <boost/lexical_cast.hpp>

#include <utils/printutils.h>
#include <utils/boost-utils.h>
#include <utils/StackCheck.h>

#include "Context.h"
#include "Expression.h"
#include "EvaluationSession.h"
#include "Filename.h"

#include <core/value/double_converter_params.h>
#include "Value.h"

#include <core/value/VectorType.h>
#include <core/value/EmbeddedVectorType.h>
#include <core/value/ValuePtr.h>
#include <core/value/ModuleReference.h>
#include <core/value/RangeType.h>
#include <core/value/FunctionType.h>
#include <core/value/ObjectType.h>
#include <core/value/UndefType.h>
#include <core/value/str_utf8_wrapper.h>
#include "QuotedString.h"

namespace fs = boost::filesystem;

using dcParams = doubleConversionParams;

const Value Value::undefined;

std::ostream& operator<<(std::ostream& stream, const Value& value) {
 if (value.type() == Value::Type::STRING) stream << QuotedString(value.toString());
 else stream << value.toString();
 return stream;
}

 namespace {
   int constexpr trimTrailingZeroesDone = 0;
   int constexpr trimTrailingZeroesContinue = 1;

   //process parameter buffer from the end to start to find out where the zeroes are located (if any).
   //parameter pos shall be the pos in buffer where '\0' is located.
   //parameter currentpos shall be set to end of buffer (where '\0' is located).
   //set parameters exppos and decimalpos when needed.
   //leave parameter zeropos as is.
   inline int
   trimTrailingZeroesHelper(char *buffer, const int pos, char *currentpos = nullptr,
   char *exppos = nullptr, char *decimalpos = nullptr, char *zeropos = nullptr)
   {
     int cont = trimTrailingZeroesContinue;
     //we have exhausted all positions from end to start
     if (currentpos <= buffer) return trimTrailingZeroesDone;
     //we do no need to process the terminator of string
     if (*currentpos == '\0') {
       currentpos--;
       cont = trimTrailingZeroesHelper(buffer, pos, currentpos, exppos, decimalpos, zeropos);
     }
     //we have an exponent and jumps to the position before the exponent - no need to process the characters belonging to the exponent
     if (cont && exppos && currentpos >= exppos) {
       currentpos = exppos;
       currentpos--;
       cont = trimTrailingZeroesHelper(buffer, pos, currentpos, exppos, decimalpos, zeropos);
     }
     //we are still on the right side of the decimal and still counting zeroes (keep track of) from the back to start
     if (cont && currentpos && decimalpos < currentpos && *currentpos == '0') {
       zeropos = currentpos;
       currentpos--;
       cont = trimTrailingZeroesHelper(buffer, pos, currentpos, exppos, decimalpos, zeropos);
     }
     // we have found the first occurrence of not a zero and have zeroes and exponent to take care
     // of (move exponent to either the position of the zero or the decimal)
     if (cont && zeropos && exppos) {
       int count = &buffer[pos] - exppos + 1;
       memmove(zeropos - 1 == decimalpos ? decimalpos : zeropos, exppos, count);
       return trimTrailingZeroesDone;
     }

     //we have found a zero and need to take care of (truncate the string to the position of either the zero or the decimal)
     if (cont && zeropos) {
       zeropos - 1 == decimalpos ? *decimalpos = '\0' : *zeropos = '\0';
       return trimTrailingZeroesDone;
     }

     //we have just another character (other than a zero) and are done
     if (cont && !zeropos)
       return trimTrailingZeroesDone;

     return trimTrailingZeroesDone;
   }

   inline void trimTrailingZeroes(char *buffer, const int pos) {
     char *decimal = strchr(buffer, '.');
     if (decimal) {
       char *exppos = strchr(buffer, dcParams::exp);
       trimTrailingZeroesHelper(buffer, pos, &buffer[pos], exppos, decimal, nullptr);
     }
   }

   inline bool HandleSpecialValues(const double& value, double_conversion::StringBuilder& builder) {
     double_conversion::Double double_inspect(value);
     if (double_inspect.IsInfinite()) {
       if (value < 0) {
         builder.AddCharacter('-');
       }
       builder.AddString(dcParams::inf);
       return true;
     }
     if (double_inspect.IsNan()) {
       builder.AddString(dcParams::nan);
       return true;
     }
     return false;
   }
} // ~namespace

std::string DoubleConvert(const double& value, char *buffer,
                                 double_conversion::StringBuilder& builder,
      const double_conversion::DoubleToStringConverter& dc) {
  builder.Reset();
  if (double_conversion::Double(value).IsSpecial()) {
    HandleSpecialValues(value, builder);
    builder.Finalize();
    return buffer;
  }
  dc.ToPrecision(value, dcParams::precisionRequested, &builder);
  int pos = builder.position(); // get position before Finalize destroys it
  builder.Finalize();
  trimTrailingZeroes(buffer, pos);
  return buffer;
}

namespace {

   static uint32_t convert_to_uint32(const double d)
   {
     auto ret = std::numeric_limits<uint32_t>::max();
     if (std::isfinite(d)) {
       try {
         ret = boost::numeric_cast<uint32_t>(d);
       } catch (boost::bad_numeric_cast&) {
         // ignore, leaving the default max() value
       }
     }
     return ret;
   }
} //~namespace

Value Value::clone() const {
  switch (this->type()) {
  case Type::UNDEFINED: return Value();
  case Type::BOOL:      return std::get<bool>(this->value);
  case Type::NUMBER:    return std::get<double>(this->value);
  case Type::STRING:    return std::get<str_utf8_wrapper>(this->value).clone();
  case Type::RANGE:     return std::get<RangePtr>(this->value).clone();
  case Type::VECTOR:    return std::get<VectorType>(this->value).clone();
  case Type::OBJECT:    return std::get<ObjectType>(this->value).clone();
  case Type::FUNCTION:  return std::get<FunctionPtr>(this->value).clone();
  case Type::MODULE :   return std::get<ModuleReferencePtr>(this->value).clone();

  default: assert(false && "unknown Value variant type"); return Value();
  }
}

Value Value::undef(const std::string& why)
{
  return Value{UndefType{why}};
}

std::string Value::typeName(Type type)
{
  switch (type) {
  case Type::UNDEFINED: return "undefined";
  case Type::BOOL:      return "bool";
  case Type::NUMBER:    return "number";
  case Type::STRING:    return "string";
  case Type::VECTOR:    return "vector";
  case Type::RANGE:     return "range";
  case Type::OBJECT:    return "object";
  case Type::FUNCTION:  return "function";
  case Type::MODULE:    return "module";
  default: assert(false && "unknown Value variant type"); return "<unknown>";
  }
}

const std::string Value::typeName() const
{
  return typeName(this->type());
}

// free functions for use by static_visitor templated functions in creating undef messages.
std::string getTypeName(const UndefType&) { return "undefined"; }
std::string getTypeName(bool) { return "bool"; }
std::string getTypeName(double) { return "number"; }
std::string getTypeName(const str_utf8_wrapper&) { return "string"; }
std::string getTypeName(const VectorType&) { return "vector"; }
std::string getTypeName(const ObjectType&) { return "object"; }
std::string getTypeName(const RangePtr&) { return "range"; }
std::string getTypeName(const FunctionPtr&) { return "function"; }
std::string getTypeName(const ModuleReferencePtr&) { return "module"; }

bool Value::toBool() const
{
  switch (this->type()) {
  case Type::UNDEFINED: return false;
  case Type::BOOL:      return std::get<bool>(this->value);
  case Type::NUMBER:    return std::get<double>(this->value) != 0;
  case Type::STRING:    return !std::get<str_utf8_wrapper>(this->value).empty();
  case Type::VECTOR:    return !std::get<VectorType>(this->value).empty();
  case Type::RANGE:     return true;
  case Type::OBJECT:    return true;
  case Type::FUNCTION:  return true;
  case Type::MODULE:  return true;
  default: assert(false && "unknown Value variant type"); return false;
  }
}

double Value::toDouble() const
{
  const double *d = std::get_if<double>(&this->value);
  return d ? *d : 0.0;
}

bool Value::getDouble(double& v) const
{
  const double *d = std::get_if<double>(&this->value);
  if (d) {
    v = *d;
    return true;
  }
  return false;
}

bool Value::getFiniteDouble(double& v) const
{
  double result;
  if (getDouble(result) && std::isfinite(result)) {
    v = result;
    return true;
  }
  return false;
}

bool Value::getUnsignedInt(unsigned int& v) const
{
  double result;
  if (getFiniteDouble(result) &&
      result >= 0.0 && result <= std::numeric_limits<unsigned int>::max())
  {
    v = result;
    return true;
  }
  return false;
}

bool Value::getPositiveInt(unsigned int& v) const
{
  double result;
  if (getFiniteDouble(result) &&
      result >= 1 && result <= std::numeric_limits<unsigned int>::max())
  {
    v = result;
    return true;
  }
  return false;
}

const str_utf8_wrapper& Value::toStrUtf8Wrapper() const {
  return std::get<str_utf8_wrapper>(this->value);
}


// Optimization to avoid multiple stream instantiations and copies to str for long vectors.
// Functions identically to "class tostring_visitor", except outputting to stream and not returning strings
class tostream_visitor
{
public:
  std::ostringstream& stream;
  mutable char buffer[dcParams::bufferSize];
  mutable double_conversion::StringBuilder builder;
  dcParams dc;
  tostream_visitor(std::ostringstream& stream)
    : stream(stream), builder(buffer, dcParams::bufferSize)
    {}

  template <typename T> void operator()(const T& op1) const {
    //std::cout << "[generic tostream_visitor]\n";
    stream << boost::lexical_cast<std::string>(op1);
  }

  void operator()(const double& op1) const {
    stream << DoubleConvert(op1, buffer, builder, dc);
  }

  void operator()(const UndefType&) const {
    stream << "undef";
  }

  void operator()(const bool& v) const {
    stream << (v ? "true" : "false");
  }

  void operator()(const EmbeddedVectorType&) const {
    assert(false && "Error: unexpected visit to EmbeddedVectorType!");
  }

  void operator()(const VectorType& v) const {
    if (StackCheck::inst().check()) {
      throw VectorEchoStringException::create();
    }
    stream << '[';
    if (!v.empty()) {
      auto it = v.begin();
      std::visit(*this, it->getVariant());
      for (++it; it != v.end(); ++it) {
        stream << ", ";
        std::visit(*this, it->getVariant());
      }
    }
    stream << ']';
  }

  void operator()(const str_utf8_wrapper& v) const {
    stream << '"' << v.toString() << '"';
  }

  void operator()(const RangePtr& v) const {
    stream << *v;
  }

  void operator()(const FunctionPtr& v) const {
    stream << *v;
  }

  void operator()(const ModuleReferencePtr& v) const {
    stream << *v;
  }
};

class tostring_visitor
{
public:
  template <typename T> std::string operator()(const T& op1) const {
    assert(false && "unhandled tostring_visitor type");
    return STR(op1);
  }

  std::string operator()(const str_utf8_wrapper& op1) const {
    return op1.toString();
  }

  std::string operator()(const double& op1) const {
    char buffer[dcParams::bufferSize];
    double_conversion::StringBuilder builder(buffer, dcParams::bufferSize);
    dcParams dc;
/*
    double_conversion::DoubleToStringConverter dc(
      dcParams::flags,
      dcParams::inf,
      dcParams::nan,
      dcParams::exp,
      dcParams::decimalLowExp,
      dcParams::decimalHighExp,
      dcParams::maxLeadingZeroes,
      dcParams::maxTrailingZeroes
    );
*/
    return DoubleConvert(op1, buffer, builder, dc);
  }

  std::string operator()(const UndefType&) const {
    return "undef";
  }

  std::string operator()(const bool& v) const {
    return v ? "true" : "false";
  }

  std::string operator()(const EmbeddedVectorType&) const {
    assert(false && "Error: unexpected visit to EmbeddedVectorType!");
    return "";
  }

  std::string operator()(const VectorType& v) const {
    // Create a single stream and pass reference to it for list elements for optimization.
    std::ostringstream stream;
    try {
      (tostream_visitor(stream))(v);
    } catch (EvaluationException& e) {
      LOG(message_group::Error, Location::NONE, "", e.what());
      throw;
    }
    return stream.str();
  }

  std::string operator()(const ObjectType& v) const {
    return STR(v);
  }

  std::string operator()(const RangePtr& v) const {
    return STR(*v);
  }

  std::string operator()(const FunctionPtr& v) const {
    return STR(*v);
  }

  std::string operator()(const ModuleReferencePtr& v) const {
    return STR(*v);
  }
};

std::string Value::toString() const
{
  return std::visit(tostring_visitor(), this->value);
}

std::string Value::toEchoString() const
{
  if (type() == Value::Type::STRING) {
    return std::string("\"") + toString() + '"';
  } else {
    return toString();
  }
}

std::string Value::toEchoStringNoThrow() const
{
  std::string ret;
  try{
    ret = toEchoString();
  } catch (EvaluationException& e) {
     ret = "...";
  }
  return ret;
}

const UndefType& Value::toUndef()
{
  return std::get<UndefType>(this->value);
}

std::string Value::toUndefString() const
{
  return std::get<UndefType>(this->value).toString();
}

class chr_visitor
{
public:
  template <typename S> std::string operator()(const S&) const
  {
    return "";
  }

  std::string operator()(const double& v) const
  {
    char buf[8];
    memset(buf, 0, 8);
    if (v > 0) {
      const gunichar c = v;
      if (g_unichar_validate(c) && (c != 0)) {
        g_unichar_to_utf8(c, buf);
      }
    }
    return std::string(buf);
  }

  std::string operator()(const VectorType& v) const
  {
    std::ostringstream stream;
    for (auto& val : v) {
      stream << val.chrString();
    }
    return stream.str();
  }

  std::string operator()(const RangePtr& v) const
  {
    const uint32_t steps = v->numValues();
    if (steps >= RangeType::MAX_RANGE_STEPS) {
      LOG(message_group::Warning, Location::NONE, "",
       "Bad range parameter in for statement: too many elements (%1$lu).", steps);
      return "";
    }

    std::ostringstream stream;
    for (double d : *v) stream << Value(d).chrString();
    return stream.str();
  }
};

std::string Value::chrString() const
{
  return std::visit(chr_visitor(), this->value);
}

const VectorType& Value::toVector() const
{
  static const VectorType empty(nullptr);
  const VectorType *v = std::get_if<VectorType>(&this->value);
  return v ? *v : empty;
}

VectorType& Value::toVectorNonConst()
{
  return std::get<VectorType>(this->value);
}

const ObjectType& Value::toObject() const
{
  static const ObjectType empty(nullptr);
  const ObjectType *v = std::get_if<ObjectType>(&this->value);
  return v ? *v : empty;
}

EmbeddedVectorType& Value::toEmbeddedVectorNonConst()
{
  return std::get<EmbeddedVectorType>(this->value);
}

const EmbeddedVectorType& Value::toEmbeddedVector() const
{
  return std::get<EmbeddedVectorType>(this->value);
}

bool Value::getVec2(double& x, double& y, bool ignoreInfinite) const
{
  if (this->type() != Type::VECTOR) return false;
  const auto& v = this->toVector();
  if (v.size() != 2) return false;
  double rx, ry;
  bool valid = ignoreInfinite
    ? v[0].getFiniteDouble(rx) && v[1].getFiniteDouble(ry)
    : v[0].getDouble(rx) && v[1].getDouble(ry);
  if (valid) {
    x = rx;
    y = ry;
  }
  return valid;
}

bool Value::getVec3(double& x, double& y, double& z) const
{
  if (this->type() != Type::VECTOR) return false;
  const VectorType& v = this->toVector();
  if (v.size() != 3) return false;
  return (v[0].getDouble(x) && v[1].getDouble(y) && v[2].getDouble(z));
}

bool Value::getVec3(double& x, double& y, double& z, double defaultval) const
{
  if (this->type() != Type::VECTOR) return false;
  const VectorType& v = toVector();
  if (v.size() == 2) {
    getVec2(x, y);
    z = defaultval;
    return true;
  } else {
    if (v.size() != 3) return false;
  }
  return (v[0].getDouble(x) && v[1].getDouble(y) && v[2].getDouble(z));
}

const RangeType& Value::toRange() const
{
  const RangePtr *val = std::get_if<RangePtr>(&this->value);
  if (val) {
    return **val;
  } else return RangeType::EMPTY;
}

const FunctionType& Value::toFunction() const
{
  return *std::get<FunctionPtr>(this->value);
}

bool Value::isUncheckedUndef() const
{
  return this->type() == Type::UNDEFINED && !std::get<UndefType>(this->value).empty();
}


class notequal_visitor
{
public:
  template <typename T, typename U> Value operator()(const T& op1, const U& op2) const { return true; }
  template <typename T> Value operator()(const T& op1, const T& op2) const { return op1 != op2; }
  Value operator()(const UndefType&, const UndefType&) const { return false; }
  template <typename T> Value operator()(const ValuePtr<T>& op1, const ValuePtr<T>& op2) const { return *op1 != *op2; }
};

class equals_visitor
{
public:
  template <typename T, typename U> Value operator()(const T& op1, const U& op2) const { return false; }
  template <typename T> Value operator()(const T& op1, const T& op2) const { return op1 == op2; }
  Value operator()(const UndefType&, const UndefType&) const { return true; }
  template <typename T> Value operator()(const ValuePtr<T>& op1, const ValuePtr<T>& op2) const { return *op1 == *op2; }
};

class less_visitor
{
public:
  template <typename T, typename U> Value operator()(const T& op1, const U& op2) const {
    return Value::undef(STR("undefined operation (", getTypeName(op1), " < ", getTypeName(op2), ")"));
  }
  template <typename T> Value operator()(const T& op1, const T& op2) const { return op1 < op2; }
  template <typename T> Value operator()(const ValuePtr<T>& op1, const ValuePtr<T>& op2) const { return *op1 < *op2; }
};

class greater_visitor
{
public:
  template <typename T, typename U> Value operator()(const T& op1, const U& op2) const {
    return Value::undef(STR("undefined operation (", getTypeName(op1), " > ", getTypeName(op2), ")"));
  }
  template <typename T> Value operator()(const T& op1, const T& op2) const { return op1 > op2; }
  template <typename T> Value operator()(const ValuePtr<T>& op1, const ValuePtr<T>& op2) const { return *op1 > *op2; }
};

class lessequal_visitor
{
public:
  template <typename T, typename U> Value operator()(const T& op1, const U& op2) const {
    return Value::undef(STR("undefined operation (", getTypeName(op1), " <= ", getTypeName(op2), ")"));
  }
  template <typename T> Value operator()(const T& op1, const T& op2) const { return op1 <= op2; }
  template <typename T> Value operator()(const ValuePtr<T>& op1, const ValuePtr<T>& op2) const { return *op1 <= *op2; }
};

class greaterequal_visitor
{
public:
  template <typename T, typename U> Value operator()(const T& op1, const U& op2) const {
    return Value::undef(STR("undefined operation (", getTypeName(op1), " >= ", getTypeName(op2), ")"));
  }
  template <typename T> Value operator()(const T& op1, const T& op2) const { return op1 >= op2; }
  template <typename T> Value operator()(const ValuePtr<T>& op1, const ValuePtr<T>& op2) const { return *op1 >= *op2; }
};

Value Value::operator==(const Value& v) const
{
  return std::visit(equals_visitor(), this->value, v.value);
}

Value Value::operator!=(const Value& v) const
{
  return std::visit(notequal_visitor(), this->value, v.value);
}

Value Value::operator<(const Value& v) const
{
  return std::visit(less_visitor(), this->value, v.value);
}

Value Value::operator>=(const Value& v) const
{
  return std::visit(greaterequal_visitor(), this->value, v.value);
}

Value Value::operator>(const Value& v) const
{
  return std::visit(greater_visitor(), this->value, v.value);
}

Value Value::operator<=(const Value& v) const
{
  return std::visit(lessequal_visitor(), this->value, v.value);
}

bool Value::cmp_less(const Value& v1, const Value& v2) {
  return v1.operator<(v2).toBool();
}

class plus_visitor
{
public:
  template <typename T, typename U> Value operator()(const T& op1, const U& op2) const {
    return Value::undef(STR("undefined operation (", getTypeName(op1), " + ", getTypeName(op2), ")"));
  }

  Value operator()(const double& op1, const double& op2) const {
    return op1 + op2;
  }

  Value operator()(const VectorType& op1, const VectorType& op2) const {
    VectorType sum(op1.evaluation_session());
    // FIXME: should we really truncate to shortest vector here?
    //   Maybe better to either "add zeroes" and return longest
    //   and/or issue an warning/error about length mismatch.
    for (auto it1 = op1.begin(), end1 = op1.end(), it2 = op2.begin(), end2 = op2.end();
         it1 != end1 && it2 != end2;
         ++it1, ++it2) {
      sum.emplace_back(*it1 + *it2);
    }
    return std::move(sum);
  }
};

Value Value::operator+(const Value& v) const
{
  return std::visit(plus_visitor(), this->value, v.value);
}

class minus_visitor
{
public:
  template <typename T, typename U> Value operator()(const T& op1, const U& op2) const {
    return Value::undef(STR("undefined operation (", getTypeName(op1), " - ", getTypeName(op2), ")"));
  }

  Value operator()(const double& op1, const double& op2) const {
    return op1 - op2;
  }

  Value operator()(const VectorType& op1, const VectorType& op2) const {
    VectorType sum(op1.evaluation_session());
    for (size_t i = 0; i < op1.size() && i < op2.size(); ++i) {
      sum.emplace_back(op1[i] - op2[i]);
    }
    return std::move(sum);
  }
};

Value Value::operator-(const Value& v) const
{
  return std::visit(minus_visitor(), this->value, v.value);
}

Value multvecnum(const VectorType& vecval, const Value& numval)
{
  // Vector * Number
  VectorType dstv(vecval.evaluation_session());
  for (const auto& val : vecval) {
    dstv.emplace_back(val * numval);
  }
  return std::move(dstv);
}

Value multmatvec(const VectorType& matrixvec, const VectorType& vectorvec)
{
  // Matrix * Vector
  VectorType dstv(matrixvec.evaluation_session());
  for (size_t i = 0; i < matrixvec.size(); ++i) {
    if (matrixvec[i].type() != Value::Type::VECTOR ||
        matrixvec[i].toVector().size() != vectorvec.size()) {
      return Value::undef(STR("Matrix must be rectangular. Problem at row ", i));
    }
    double r_e = 0.0;
    for (size_t j = 0; j < matrixvec[i].toVector().size(); ++j) {
      if (matrixvec[i].toVector()[j].type() != Value::Type::NUMBER) {
        return Value::undef(STR("Matrix must contain only numbers. Problem at row ", i, ", col ", j));
      }
      if (vectorvec[j].type() != Value::Type::NUMBER) {
        return Value::undef(STR("Vector must contain only numbers. Problem at index ", j));
      }
      r_e += matrixvec[i].toVector()[j].toDouble() * vectorvec[j].toDouble();
    }
    dstv.emplace_back(Value(r_e));
  }
  return std::move(dstv);
}

Value multvecmat(const VectorType& vectorvec, const VectorType& matrixvec)
{
  assert(vectorvec.size() == matrixvec.size());
  // Vector * Matrix
  VectorType dstv(matrixvec[0].toVector().evaluation_session());
  size_t firstRowSize = matrixvec[0].toVector().size();
  for (size_t i = 0; i < firstRowSize; ++i) {
    double r_e = 0.0;
    for (size_t j = 0; j < vectorvec.size(); ++j) {
      if (matrixvec[j].type() != Value::Type::VECTOR ||
          matrixvec[j].toVector().size() != firstRowSize) {
        LOG(message_group::Warning, Location::NONE, "", "Matrix must be rectangular. Problem at row %1$lu", j);
        return Value::undef(STR("Matrix must be rectangular. Problem at row ", j));
      }
      if (vectorvec[j].type() != Value::Type::NUMBER) {
        LOG(message_group::Warning, Location::NONE, "", "Vector must contain only numbers. Problem at index %1$lu", j);
        return Value::undef(STR("Vector must contain only numbers. Problem at index ", j));
      }
      if (matrixvec[j].toVector()[i].type() != Value::Type::NUMBER) {
        LOG(message_group::Warning, Location::NONE, "", "Matrix must contain only numbers. Problem at row %1$lu, col %2$lu", j, i);
        return Value::undef(STR("Matrix must contain only numbers. Problem at row ", j, ", col ", i));
      }
      r_e += vectorvec[j].toDouble() * matrixvec[j].toVector()[i].toDouble();
    }
    dstv.emplace_back(r_e);
  }
  return Value(std::move(dstv));
}

Value multvecvec(const VectorType& vec1, const VectorType& vec2) {
  // Vector dot product.
  auto r = 0.0;
  for (size_t i = 0; i < vec1.size(); i++) {
    if (vec1[i].type() != Value::Type::NUMBER || vec2[i].type() != Value::Type::NUMBER) {
      return Value::undef(STR("undefined operation (", vec1[i].typeName(), " * ", vec2[i].typeName(), ")"));
    }
    r += vec1[i].toDouble() * vec2[i].toDouble();
  }
  return Value(r);
}

class multiply_visitor
{
public:
  template <typename T, typename U> Value operator()(const T& op1, const U& op2) const {
    return Value::undef(STR("undefined operation (", getTypeName(op1), " * ", getTypeName(op2), ")"));
  }
  Value operator()(const double& op1, const double& op2) const { return op1 * op2; }
  Value operator()(const double& op1, const VectorType& op2) const { return multvecnum(op2, op1); }
  Value operator()(const VectorType& op1, const double& op2) const { return multvecnum(op1, op2); }

  Value operator()(const VectorType& op1, const VectorType& op2) const {
    if (op1.empty() || op2.empty()) return Value::undef("Multiplication is undefined on empty vectors");
    auto first1 = op1.begin(), first2 = op2.begin();
    auto eltype1 = (*first1).type(), eltype2 = (*first2).type();
    if (eltype1 == Value::Type::NUMBER) {
      if (eltype2 == Value::Type::NUMBER) {
        if (op1.size() == op2.size()) return multvecvec(op1, op2);
        else return Value::undef(STR("vector*vector requires matching lengths (", op1.size(), " != ", op2.size(), ')'));
      } else if (eltype2 == Value::Type::VECTOR) {
        if (op1.size() == op2.size()) return multvecmat(op1, op2);
        else return Value::undef(STR("vector*matrix requires vector length to match matrix row count (", op1.size(), " != ", op2.size(), ')'));
      }
    } else if (eltype1 == Value::Type::VECTOR) {
      if (eltype2 == Value::Type::NUMBER) {
        if ((*first1).toVector().size() == op2.size()) return multmatvec(op1, op2);
        else return Value::undef(STR("matrix*vector requires matrix column count to match vector length (", (*first1).toVector().size(), " != ", op2.size(), ')'));
      } else if (eltype2 == Value::Type::VECTOR) {
        if ((*first1).toVector().size() == op2.size()) {
          // Matrix * Matrix
          VectorType dstv(op1.evaluation_session());
          size_t i = 0;
          for (const auto& srcrow : op1) {
            const auto& srcrowvec = srcrow.toVector();
            if (srcrowvec.size() != op2.size()) return Value::undef(STR("matrix*matrix left operand row length does not match right operand row count (", srcrowvec.size(), " != ", op2.size(), ") at row ", i));
            auto temp = multvecmat(srcrowvec, op2);
            if (temp.isUndefined()) {
              temp.toUndef().append(STR("while processing left operand at row ", i));
              return temp;
            } else {
              dstv.emplace_back(std::move(temp));
            }
            ++i;
          }
          return Value(std::move(dstv));
        } else {
          return Value::undef(STR("matrix*matrix requires left operand column count to match right operand row count (",
          (*first1).toVector().size(), " != ", op2.size(), ')'));
        }
      }
    }
    return Value::undef(STR("undefined vector*vector multiplication where first elements are types ",
       (*first1).typeName(), " and ", (*first2).typeName() ));
  }
};

Value Value::operator*(const Value& v) const
{
  return std::visit(multiply_visitor(), this->value, v.value);
}

Value Value::operator/(const Value& v) const
{
  if (this->type() == Type::NUMBER && v.type() == Type::NUMBER) {
    return this->toDouble() / v.toDouble();
  } else if (this->type() == Type::VECTOR && v.type() == Type::NUMBER) {
    VectorType dstv(this->toVector().evaluation_session());
    for (const auto& vecval : this->toVector()) {
      dstv.emplace_back(vecval / v);
    }
    return std::move(dstv);
  } else if (this->type() == Type::NUMBER && v.type() == Type::VECTOR) {
    VectorType dstv(v.toVector().evaluation_session());
    for (const auto& vecval : v.toVector()) {
      dstv.emplace_back(*this / vecval);
    }
    return std::move(dstv);
  }
  return Value::undef(STR("undefined operation (", this->typeName(), " / ", v.typeName(), ")"));
}

Value Value::operator%(const Value& v) const
{
  if (this->type() == Type::NUMBER && v.type() == Type::NUMBER) {
    return fmod(std::get<double>(this->value), std::get<double>(v.value));
  }
  return Value::undef(STR("undefined operation (", this->typeName(), " % ", v.typeName(), ")"));
}

Value Value::operator-() const
{
  if (this->type() == Type::NUMBER) {
    return Value(-this->toDouble());
  } else if (this->type() == Type::VECTOR) {
    VectorType dstv(this->toVector().evaluation_session());
    for (const auto& vecval : this->toVector()) {
      dstv.emplace_back(-vecval);
    }
    return std::move(dstv);
  }
  return Value::undef(STR("undefined operation (-", this->typeName(), ")"));
}

Value Value::operator^(const Value& v) const
{
  if (this->type() == Type::NUMBER && v.type() == Type::NUMBER) {
    return {pow(std::get<double>(this->value), std::get<double>(v.value))};
  }
  return Value::undef(STR("undefined operation (", this->typeName(), " ^ ", v.typeName(), ")"));
}

/*
 * bracket operation [] detecting multi-byte unicode.
 * If the string is multi-byte unicode then the index will offset to the character (2 or 4 byte) and not to the byte.
 * A 'normal' string with byte chars are a subset of unicode and still work.
 */
class bracket_visitor{
public:
  Value operator()(const str_utf8_wrapper& str, const double& idx) const {
    const auto i = convert_to_uint32(idx);
    if (i < str.size()) {
      // Ensure character (not byte) index is inside the character/glyph array
      if (glong(i) < str.get_utf8_strlen()) {
        gchar utf8_of_cp[6] = ""; //A buffer for a single unicode character to be copied into
        auto ptr = g_utf8_offset_to_pointer(str.c_str(), i);
        if (ptr) {
          g_utf8_strncpy(utf8_of_cp, ptr, 1);
        }
        return std::string(utf8_of_cp);
      }
    }
    return Value::undefined.clone();
  }

  Value operator()(const VectorType& vec, const double& idx) const
  {
    const auto i = convert_to_uint32(idx);
    if (i < vec.size()) return vec[i].clone();
    return Value::undef(STR("index ", i, " out of bounds for vector of size ", vec.size()));
  }

  Value operator()(const ObjectType& obj, const str_utf8_wrapper& key) const {
    return obj[key].clone();
  }

  Value operator()(const RangePtr& range, const double& idx) const {
    const auto i = convert_to_uint32(idx);
    switch (i) {
    case 0: return range->begin_value();
    case 1: return range->step_value();
    case 2: return range->end_value();
    }
    return Value::undef("subscript operator only defined for indices 0-2 on range (begin,step,end)");
  }

  template <typename T, typename U> Value operator()(const T& op1, const U& op2) const {
    //std::cout << "generic bracket_visitor " << getTypeName(op1) << " " << getTypeName(op2) << "\n";
    return Value::undef(STR("undefined operation ", getTypeName(op1), "[", getTypeName(op2), "]"));
  }
};

Value Value::operator[](const Value& v) const
{
  return std::visit(bracket_visitor(), this->value, v.value);
}

Value Value::operator[](size_t idx) const
{
  Value v{(double)idx};
  return std::visit(bracket_visitor(), this->value, v.value);
}

size_t str_utf8_wrapper::iterator::char_len()
{
  return g_utf8_next_char(ptr) - ptr;
}
