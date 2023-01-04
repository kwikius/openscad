#pragma once

#include <vector>
#include <memory>

class EmbeddedVectorType;
class Value;

  /**
   * VectorType is the underlying "BoundedType" of std::variant for OpenSCAD vectors.
   * It holds only a shared_ptr to its VectorObject type, and provides a convenient
   * interface for various operations needed on the vector.
   *
  **/

class VectorType
{

protected:
 // The object type which VectorType's shared_ptr points to.
 struct VectorObject {
   using vec_t = std::vector<Value>;
   using size_type = vec_t::size_type;
   vec_t vec;
   size_type embed_excess = 0; // Keep count of the number of embedded elements *excess of* vec.size()
   class EvaluationSession *evaluation_session = nullptr; // Used for heap size bookkeeping. May be null for vectors of known small maximum size.
   size_type size() const { return vec.size() + embed_excess;  }
 };
 using vec_t = VectorObject::vec_t;
public:
 std::shared_ptr<VectorObject> ptr;
protected:

 // A Deleter is used on the shared_ptrs to avoid stack overflow in cases
 // of destructing a very large list of nested embedded vectors, such as from a
 // recursive function which concats one element at a time.
 // (A similar solution can also be seen with CSGNode.h:CSGOperationDeleter).
 struct VectorObjectDeleter {
   void operator()(VectorObject *vec);
 };
 void flatten() const; // flatten replaces VectorObject::vec with a new vector
                       // where any embedded elements are copied directly into the top level vec,
                       // leaving only true elements for straightforward indexing by operator[].
 explicit VectorType(const std::shared_ptr<VectorObject>& copy) : ptr(copy) { } // called by clone()
public:
 using size_type = VectorObject::size_type;
 static const VectorType EMPTY;
 // EmbeddedVectorType-aware iterator, manages its own stack of begin/end vec_t::const_iterators
 // such that calling code will only receive references to "true" elements (i.e. NOT EmbeddedVectorTypes).
 // Also tracks the overall element index. In case flattening occurs during iteration, it can continue based on that index.
// (Issue #3541)
 class iterator
 {
private:
   const VectorObject *vo;
   std::vector<std::pair<vec_t::const_iterator, vec_t::const_iterator>> it_stack;
   vec_t::const_iterator it, end;
   size_t index;

   // Recursively push stack while current (pseudo)element is an EmbeddedVector
   //  - Depends on the fact that VectorType::emplace_back(EmbeddedVectorType&& mbed)
   //    will not embed an empty vector, which ensures iterator will arrive at an actual element,
   //    unless already at end of parent VectorType.
   void check_and_push();
public:
   using iterator_category = std::forward_iterator_tag;
   using value_type = Value;
   using difference_type = void;
   using reference = const value_type&;
   using pointer = const value_type *;

   iterator() : vo(EMPTY.ptr.get()), it_stack(), it(EMPTY.ptr->vec.begin()), end(EMPTY.ptr->vec.end()), index(0) {}
   iterator(const VectorObject *v) : vo(v), it(v->vec.begin()), end(v->vec.end()), index(0) {
     if (vo->embed_excess) check_and_push();
   }
   iterator(const VectorObject *v, bool /*end*/) : vo(v), index(v->size()) { }
   iterator& operator++() {
     ++index;
     if (vo->embed_excess) {
       // recursively increment and pop stack while at the end of EmbeddedVector(s)
       while (++it == end && !it_stack.empty()) {
         const auto& up = it_stack.back();
         it = up.first;
         end = up.second;
         it_stack.pop_back();
       }
       check_and_push();
     } else { // vo->vec is flat
       it = vo->vec.begin() + index;
     }
     return *this;
   }
   reference operator*() const { return *it; }
   pointer operator->() const { return &*it; }
   bool operator==(const iterator& other) const { return this->vo == other.vo && this->index == other.index; }
   bool operator!=(const iterator& other) const { return this->vo != other.vo || this->index != other.index; }
 }; // ~iterator

 using const_iterator = const iterator;
 VectorType(class EvaluationSession *session); // : ptr(shared_ptr<VectorObject>(new VectorObject(), VectorObjectDeleter() )) {}
 VectorType(class EvaluationSession *session, double x, double y, double z);
 VectorType(const VectorType&) = delete; // never copy, move instead
 VectorType& operator=(const VectorType&) = delete; // never copy, move instead
 VectorType(VectorType&&) = default;
 VectorType& operator=(VectorType&&) = default;
 VectorType clone() const { return VectorType(this->ptr); } // Copy explicitly only when necessary
 static Value Empty() ;// { return VectorType(nullptr); }

 const_iterator begin() const { return iterator(ptr.get()); }
 const_iterator   end() const { return iterator(ptr.get(), true); }
 size_type size() const { return ptr->size(); }
 bool empty() const { return ptr->vec.empty(); }
 // const accesses to VectorObject require .clone to be move-able
 const Value& operator[](size_t idx) const ;
 Value operator==(const VectorType& v) const;
 Value operator<(const VectorType& v) const;
 Value operator>(const VectorType& v) const;
 Value operator!=(const VectorType& v) const;
 Value operator<=(const VectorType& v) const;
 Value operator>=(const VectorType& v) const;
 class EvaluationSession *evaluation_session() const { return ptr->evaluation_session; }

 void emplace_back(Value&& val);
 void emplace_back(EmbeddedVectorType&& mbed);
 template <typename ... Args> void emplace_back(Args&&... args)
 { emplace_back(Value(std::forward<Args>(args)...)); }
};

