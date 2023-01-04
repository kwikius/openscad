#pragma once

#include <iostream>
#include <iterator>

class RangeType
{
private:
  double begin_val;
  double step_val;
  double end_val;

public:
  static constexpr uint32_t MAX_RANGE_STEPS = 10000;
  static const RangeType EMPTY;

  enum class type_t { RANGE_TYPE_BEGIN, RANGE_TYPE_RUNNING, RANGE_TYPE_END };

  class iterator
  {
public:
    // iterator_traits required types:
    using iterator_category = std::forward_iterator_tag;
    using value_type = double;
    using difference_type = void; // type used by operator-(iterator), not implemented for forward iterator
    using reference = value_type; // type used by operator*(), not actually a reference
    using pointer = void;     // type used by operator->(), not implemented
    iterator(const RangeType& range, type_t type);
    iterator& operator++();
    reference operator*();
    bool operator==(const iterator& other) const;
    bool operator!=(const iterator& other) const;
private:
    const RangeType& range;
    double val;
    type_t type;
    const uint32_t num_values;
    uint32_t i_step;
    void update_type();
  };

  RangeType(const RangeType&) = delete;       // never copy, move instead
  RangeType& operator=(const RangeType&) = delete; // never copy, move instead
  RangeType(RangeType&&) = default;
  RangeType& operator=(RangeType&&) = default;

  explicit RangeType(double begin, double end)
    : begin_val(begin), step_val(1.0), end_val(end) {}

  explicit RangeType(double begin, double step, double end)
    : begin_val(begin), step_val(step), end_val(end) {}

  bool operator==(const RangeType& other) const {
    auto n1 = this->numValues();
    auto n2 = other.numValues();
    if (n1 == 0) return n2 == 0;
    if (n2 == 0) return false;
    return this == &other ||
           (this->begin_val == other.begin_val &&
            this->step_val == other.step_val &&
            n1 == n2);
  }

  bool operator!=(const RangeType& other) const {
    return !(*this == other);
  }

  bool operator<(const RangeType& other) const {
    auto n1 = this->numValues();
    auto n2 = other.numValues();
    if (n1 == 0) return 0 < n2;
    if (n2 == 0) return false;
    return this->begin_val < other.begin_val ||
           (this->begin_val == other.begin_val &&
            (this->step_val < other.step_val || (this->step_val == other.step_val && n1 < n2))
           );
  }

  bool operator<=(const RangeType& other) const {
    auto n1 = this->numValues();
    auto n2 = other.numValues();
    if (n1 == 0) return true; // (0 <= n2) is always true
    if (n2 == 0) return false;
    return this->begin_val < other.begin_val ||
           (this->begin_val == other.begin_val &&
            (this->step_val < other.step_val || (this->step_val == other.step_val && n1 <= n2))
           );
  }

  bool operator>(const RangeType& other) const {
    auto n1 = this->numValues();
    auto n2 = other.numValues();
    if (n2 == 0) return n1 > 0;
    if (n1 == 0) return false;
    return this->begin_val > other.begin_val ||
           (this->begin_val == other.begin_val &&
            (this->step_val > other.step_val || (this->step_val == other.step_val && n1 > n2))
           );
  }

  bool operator>=(const RangeType& other) const {
    auto n1 = this->numValues();
    auto n2 = other.numValues();
    if (n2 == 0) return true; // (n1 >= 0) is always true
    if (n1 == 0) return false;
    return this->begin_val > other.begin_val ||
           (this->begin_val == other.begin_val &&
            (this->step_val > other.step_val || (this->step_val == other.step_val && n1 >= n2))
           );
  }

  double begin_value() const { return begin_val; }
  double step_value() const { return step_val; }
  double end_value() const { return end_val; }

  iterator begin() const { return iterator(*this, type_t::RANGE_TYPE_BEGIN); }
  iterator end() const { return iterator(*this, type_t::RANGE_TYPE_END); }

  /// return number of values, max uint32_t value if step is 0 or range is infinite
  uint32_t numValues() const;
};
std::ostream& operator<<(std::ostream& stream, const RangeType& r);


