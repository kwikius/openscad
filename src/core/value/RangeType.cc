#include <cmath>

#include <core/Value.h>
#include "double_converter_params.h"

using dcParams = doubleConversionParams;

const RangeType RangeType::EMPTY{0, 0, 0};

uint32_t RangeType::numValues() const
{
  if (std::isnan(begin_val) || std::isnan(end_val) || std::isnan(step_val)) {
    return 0;
  }
  if (step_val < 0) {
    if (begin_val < end_val) return 0;
  } else {
    if (begin_val > end_val) return 0;
  }
  if ((begin_val == end_val) || std::isinf(step_val)) {
    return 1;
  }
  if (std::isinf(begin_val) || std::isinf(end_val) || step_val == 0) {
    return std::numeric_limits<uint32_t>::max();
  }
  // Use nextafter to compensate for possible floating point inaccurary where result is just below a whole number.
  const uint32_t max = std::numeric_limits<uint32_t>::max();
  uint32_t num_steps = std::nextafter((end_val - begin_val) / step_val, max);
  return (num_steps == max) ? max : num_steps + 1;
}

RangeType::iterator::iterator(const RangeType& range, type_t type) : range(range), val(range.begin_val), type(type),
  num_values(range.numValues()), i_step(type == type_t::RANGE_TYPE_END ? num_values : 0)
{
  update_type();
}

void RangeType::iterator::update_type()
{
  if (range.step_val == 0) {
    type = type_t::RANGE_TYPE_END;
  } else if (range.step_val < 0) {
    if (i_step >= num_values) {
      type = type_t::RANGE_TYPE_END;
    }
  } else {
    if (i_step >= num_values) {
      type = type_t::RANGE_TYPE_END;
    }
  }

  if (std::isnan(range.begin_val) || std::isnan(range.end_val) || std::isnan(range.step_val)) {
    type = type_t::RANGE_TYPE_END;
    i_step = num_values;
  }
}

RangeType::iterator::reference RangeType::iterator::operator*()
{
  return val;
}

RangeType::iterator& RangeType::iterator::operator++()
{
  val = range.begin_val + range.step_val * ++i_step;
  update_type();
  return *this;
}

bool RangeType::iterator::operator==(const iterator& other) const
{
  if (type == type_t::RANGE_TYPE_RUNNING) {
    return (type == other.type) && (val == other.val) && (range == other.range);
  } else {
    return (type == other.type) && (range == other.range);
  }
}

bool RangeType::iterator::operator!=(const iterator& other) const
{
  return !(*this == other);
}

std::ostream& operator<<(std::ostream& stream, const RangeType& r)
{
  char buffer[dcParams::bufferSize];
  double_conversion::StringBuilder builder(buffer, dcParams::bufferSize);
//  double_conversion::DoubleToStringConverter dc(
//    dcParams::flags, dcParams::inf,
//    dcParams::nan,
//    dcParams::exp,
//    dcParams::decimalLowExp,
//    dcParams::decimalHighExp,
//    dcParams::maxLeadingZeroes,
//    dcParams::maxTrailingZeroes
//  );
  dcParams dc;
  return stream << "["
                << DoubleConvert(r.begin_value(), buffer, builder, dc) << " : "
                << DoubleConvert(r.step_value(),  buffer, builder, dc) << " : "
                << DoubleConvert(r.end_value(),   buffer, builder, dc) << "]";
}

