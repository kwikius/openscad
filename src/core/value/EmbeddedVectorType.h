
#pragma once

#include "VectorType.h"

  /**
   * EmbeddedVectorType class derives from VectorType and enables O(1) concatenation of vectors
   * by treating their elements as elements of their parent, traversable via VectorType's custom iterator.
   * -- An embedded vector should never exist "in the wild", only as a pseudo-element of a parent vector.
   *    Eg "Lc*" Expressions return Embedded Vectors but they are necessarily child expressions of a Vector expression.
   * -- Any VectorType containing embedded elements will be forced to "flatten" upon usage of operator[],
   *    which is the only case of random-access.
   * -- Any loops through VectorTypes should prefer automatic range-based for loops  eg: for(const auto& value : vec) { ... }
   *    which make use of begin() and end() iterators of VectorType.  https://en.cppreference.com/w/cpp/language/range-for
   * -- Moving a temporary Value of type VectorType or EmbeddedVectorType is always safe,
   *    since it just moves the shared_ptr in its possession (which might be a copy but that doesn't matter).
   *    Additionally any VectorType can be converted to an EmbeddedVectorType by moving it into
   *    EmbeddedVectorType's converting constructor (or vice-versa).
   * -- HOWEVER, moving elements out of a [Embedded]VectorType is potentially DANGEROUS unless it can be
   *    verified that ( ptr.use_count() == 1 ) for that outermost [Embedded]VectorType
   *    AND recursively any EmbeddedVectorTypes which led to that element.
   *    Therefore elements are currently cloned rather than making any attempt to move.
   *    Performing such use_count checks may be an area for further optimization.
   */

  class EmbeddedVectorType : public VectorType
  {
private:
    explicit EmbeddedVectorType(const std::shared_ptr<VectorObject>& copy)
     : VectorType(copy) { } // called by clone()
public:
    EmbeddedVectorType(class EvaluationSession *session) : VectorType(session) {}
    EmbeddedVectorType(const EmbeddedVectorType&) = delete;
    EmbeddedVectorType& operator=(const EmbeddedVectorType&) = delete;
    EmbeddedVectorType(EmbeddedVectorType&&) = default;
    EmbeddedVectorType& operator=(EmbeddedVectorType&&) = default;

    EmbeddedVectorType(VectorType&& v) : VectorType(std::move(v)) {} // converting constructor
    EmbeddedVectorType clone() const { return EmbeddedVectorType(this->ptr); }
    static Value Empty() ;// { return EmbeddedVectorType(nullptr); }
  };

