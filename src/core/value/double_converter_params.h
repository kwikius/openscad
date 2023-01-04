#pragma once

#include <double-conversion/double-conversion.h>
#include <double-conversion/utils.h>
#include <double-conversion/ieee.h>

struct doubleConversionParams : double_conversion::DoubleToStringConverter{

   using Flags_t = double_conversion::DoubleToStringConverter::Flags;
   static constexpr Flags_t flags = static_cast<Flags_t>(
     static_cast<int>(Flags_t::UNIQUE_ZERO) | static_cast<int>(Flags_t::EMIT_POSITIVE_EXPONENT_SIGN)
   );
   static constexpr std::size_t bufferSize = 128U;
   static constexpr char inf[] = "inf";
   static constexpr char nan[] = "nan";
   static constexpr char exp = 'e';
   static constexpr int decimalLowExp = -6;
   static constexpr int decimalHighExp = 21;
   static constexpr int maxLeadingZeroes = 5;
   static constexpr int maxTrailingZeroes = 0;

/* WARNING: using precisionRequested > 8 will significantly slow double to string
 * conversion, defeating the purpose of using double-conversion library */
   static int constexpr precisionRequested = 6;

   doubleConversionParams()
   : double_conversion::DoubleToStringConverter(
      flags,
      inf,
      nan,
      exp,
      decimalLowExp,
      decimalHighExp,
      maxLeadingZeroes,
      maxTrailingZeroes
    ){}
};

std::string DoubleConvert(const double& value, char *buffer,
                                 double_conversion::StringBuilder& builder,
      const double_conversion::DoubleToStringConverter& dc);

