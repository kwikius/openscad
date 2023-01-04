


#include <core/Value.h>
#include <utils/printutils.h>

#include "VectorType.h"
#include <core/EvaluationSession.h>

/**
* @brief Recursively push stack while current (pseudo)element is an EmbeddedVector
*  - Depends on the fact that VectorType::emplace_back(EmbeddedVectorType&& mbed)
*   will not embed an empty vector, which ensures iterator will arrive at an actual element,
*   unless already at end of parent VectorType.
**/

const VectorType VectorType::EMPTY(nullptr);

void VectorType::iterator::check_and_push()
{
  if (it != end) {
    while (it->type() == Value::Type::EMBEDDED_VECTOR) {
      const vec_t& cur = it->toEmbeddedVector().ptr->vec;
      it_stack.emplace_back(it, end);
      it = cur.begin();
      end = cur.end();
    }
  }
}

Value VectorType::Empty() { return VectorType(nullptr); }

 const Value& VectorType::operator[](size_t idx) const {
   if (idx < this->size()) {
     if (ptr->embed_excess) flatten();
     return ptr->vec[idx];
   } else {
     return Value::undefined;
   }
 }

Value VectorType::operator==(const VectorType& v) const {
  size_t i = 0;
  auto first1 = this->begin(), last1 = this->end(), first2 = v.begin(), last2 = v.end();
  for ( ; (first1 != last1) && (first2 != last2); ++first1, ++first2, ++i) {
    Value temp = *first1 == *first2;
    if (temp.isUndefined()) {
      temp.toUndef().append(STR("in vector comparison at index ", i));
      return temp;
    }
    if (!temp.toBool()) return false;
  }
  return (first1 == last1) && (first2 == last2);
}

Value VectorType::operator!=(const VectorType& v) const {
  Value temp = this->VectorType::operator==(v);
  if (temp.isUndefined()) return temp;
  return !temp.toBool();
}

// lexicographical compare with possible undef result
Value VectorType::operator<(const VectorType& v) const {
  auto first1 = this->begin(), last1 = this->end(), first2 = v.begin(), last2 = v.end();
  size_t i = 0;
  for ( ; (first1 != last1) && (first2 != last2); ++first1, ++first2, ++i) {
    Value temp = *first1 < *first2;
    if (temp.isUndefined()) {
      temp.toUndef().append(STR("in vector comparison at index ", i));
      return temp;
    }
    if (temp.toBool()) return true;
    if ((*first2 < *first1).toBool()) return false;
  }
  return (first1 == last1) && (first2 != last2);
}

Value VectorType::operator>(const VectorType& v) const {
  return v.VectorType::operator<(*this);
}

Value VectorType::operator<=(const VectorType& v) const {
  Value temp = this->VectorType::operator>(v);
  if (temp.isUndefined()) return temp;
  return !temp.toBool();
}

Value VectorType::operator>=(const VectorType& v) const {
  Value temp = this->VectorType::operator<(v);
  if (temp.isUndefined()) return temp;
  return !temp.toBool();
}

VectorType::VectorType(EvaluationSession *session) :
  ptr(std::shared_ptr<VectorObject>(new VectorObject(), VectorObjectDeleter() ))
{
  ptr->evaluation_session = session;
}

VectorType::VectorType(class EvaluationSession *session, double x, double y, double z) :
  ptr(std::shared_ptr<VectorObject>(new VectorObject(), VectorObjectDeleter() ))
{
  ptr->evaluation_session = session;
  emplace_back(x);
  emplace_back(y);
  emplace_back(z);
}

void VectorType::emplace_back(Value&& val)
{
  if (val.type() == Value::Type::EMBEDDED_VECTOR) {
    emplace_back(std::move(val.toEmbeddedVectorNonConst()));
  } else {
    ptr->vec.push_back(std::move(val));
    if (ptr->evaluation_session) {
      ptr->evaluation_session->accounting().addVectorElement(1);
    }
  }
}

// Specialized handler for EmbeddedVectorTypes
void VectorType::emplace_back(EmbeddedVectorType&& mbed)
{
  if (mbed.size() > 1) {
    // embed_excess represents how many to add to vec.size() to get the total elements after flattening,
    // the embedded vector itself already counts towards an element in the parent's size, so subtract 1 from its size.
    ptr->embed_excess += mbed.size() - 1;
    ptr->vec.emplace_back(std::move(mbed));
    if (ptr->evaluation_session) {
      ptr->evaluation_session->accounting().addVectorElement(1);
    }
  } else if (mbed.size() == 1) {
    // If embedded vector contains only one value, then insert a copy of that element
    // Due to the above mentioned "-1" count, putting it in directaly as an EmbeddedVector
    // would not change embed_excess, which is needed to check if flatten is required.
    emplace_back(mbed.ptr->vec[0].clone());
  }
  // else mbed.size() == 0, do nothing
}

void VectorType::flatten() const
{
  vec_t ret;
  ret.reserve(this->size());
  // VectorType::iterator already handles the tricky recursive navigation of embedded vectors,
  // so just build up our new vector from that.
  for (const auto& el : *this) ret.emplace_back(el.clone());
  assert(ret.size() == this->size());
  ptr->embed_excess = 0;
  if (ptr->evaluation_session) {
    ptr->evaluation_session->accounting().addVectorElement(ret.size());
    ptr->evaluation_session->accounting().removeVectorElement(ptr->vec.size());
  }
  ptr->vec = std::move(ret);
}

void VectorType::VectorObjectDeleter::operator()(VectorObject *v)
{
  if (v->evaluation_session) {
    v->evaluation_session->accounting().removeVectorElement(v->vec.size());
  }

  VectorObject *orig = v;
  std::shared_ptr<VectorObject> curr;
  std::vector<std::shared_ptr<VectorObject>> purge;
  while (true) {
    if (v && v->embed_excess) {
      for (Value& val : v->vec) {
        auto type = val.type();
        if (type == Value::Type::EMBEDDED_VECTOR) {
          std::shared_ptr<VectorObject>& temp = std::get<EmbeddedVectorType>(val.getVariant()).ptr;
          if (temp.use_count() <= 1) purge.emplace_back(std::move(temp));
        } else if (type == Value::Type::VECTOR) {
          std::shared_ptr<VectorObject>& temp = std::get<VectorType>(val.getVariant()).ptr;
          if (temp.use_count() <= 1) purge.emplace_back(std::move(temp));
        }
      }
    }
    if (purge.empty()) break;
    curr = std::move(purge.back()); // this should cause destruction of the *previous value* for curr
    v = curr.get();
    purge.pop_back();
  }
  delete orig;
}


