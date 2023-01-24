#pragma once
#include <vector>
#include <algorithm>

#include "utils/degree_trig.h"

namespace primitives{

   struct point2d {
     double x, y;
   };

   struct point3d {
     double x, y, z;
   };

   inline constexpr double fs_Smallest = 0.01;
   inline constexpr double fa_Smallest = 0.01;

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
}
