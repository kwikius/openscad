#pragma once

#include <cmath>
#include <vector>
#include <string>
#include <algorithm>
#include <type_traits>

#include <quan/two_d/vect.hpp>
#include <quan/three_d/vect.hpp>

#include <geometry/linalg.h>
#include "utils/degree_trig.h"
#include <utils/calc.h>


class Value;
class Parameters;
class ModuleInstantiation;

template <typename T>
  requires std::is_arithmetic_v<T>
[[nodiscard]] inline bool isPositiveFinite(T const & v)
{
   return (v > 0) && std::isfinite(v);
}

template <typename T>
  requires std::is_arithmetic_v<T>
[[nodiscard]] inline bool isNonNegativeFinite(T const & v)
{
   return (v >= 0) && std::isfinite(v);
}



template <typename T, typename ... R>
[[nodiscard]] inline bool isPositiveFinite(T const & v, R const & ... r )
{
   return isPositiveFinite(v) && isPositiveFinite(r...);
}

template <typename T>
  requires std::is_arithmetic_v<T>
[[nodiscard]] inline bool isPositiveFinite(quan::three_d::vect<T> const & v)
{
   return isPositiveFinite(v.x,v.y,v.z);
}

template <typename T>
  requires std::is_arithmetic_v<T>
[[nodiscard]] inline bool isPositiveFinite(quan::two_d::vect<T> const & v)
{
   return isPositiveFinite(v.x,v.y);
}

[[nodiscard]] bool isNumber(Value const & v);
[[nodiscard]] bool isVector(Value const & v);
[[nodiscard]] bool isBool(Value const & v);
[[nodiscard]] bool isDefined(Value const & v);
[[nodiscard]] bool isUndefined(Value const & v);

namespace primitives{

   using point2d = quan::two_d::vect<double>;
   using point3d = quan::three_d::vect<double>;

   using point2di = quan::two_d::vect<int>;
   using point3di = quan::three_d::vect<int>;

   struct facet_params_t{
/**
* @brief $fn is number of fragments and usually has the default value of 0.
* When this variable has a value greater than zero, the other two
* variables are ignored, and a full circle is rendered using this
* number of fragments
**/
      double fn;
      double fs;  /// @brief $fs is the minimum size of a fragment
      double fa; /// @brief $fa is the minimum angle for a fragment.

      static constexpr double fsMin = 0.01;
      static constexpr double faMin = 0.01;
   };

   inline std::vector<point2d>
   generate_circle(double r, int fragments)
   {
      std::vector<point2d> circle(fragments);
      std::ranges::generate(circle,
         [r,fragments,i=0] () mutable {
            double const phi = (360.0 * i++) / fragments;
            return point2d{r * cos_degrees(phi),r * sin_degrees(phi)};
         }
      );
      return circle;
   }

   inline int get_fragments_from_r(double const & r, facet_params_t const & fp)
   {
      return Calc::get_fragments_from_r(r,fp.fn,fp.fs,fp.fa);
   }

   bool get_center(Parameters const & parameters,std::variant<bool, point3di>& center);

   Value lookup_radius(const Parameters& parameters, const ModuleInstantiation *inst,
     const std::string& diameter_var, const std::string& radius_var);

   void set_fragments(const Parameters& parameters,
     const ModuleInstantiation *inst, double& fn, double& fs, double& fa);

   inline void set_fragments(const Parameters& parameters,
     const ModuleInstantiation *inst, facet_params_t & fp)
   {
      set_fragments(parameters,inst,fp.fn,fp.fs,fp.fa);
   }

}//~namespace primitives
