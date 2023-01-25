#pragma once

#include <vector>
#include <string>
#include <algorithm>

#include <quan/two_d/vect.hpp>
#include <quan/three_d/vect.hpp>

#include <geometry/linalg.h>
#include "utils/degree_trig.h"
#include <utils/calc.h>

class Value;
class Parameters;
class ModuleInstantiation;

namespace primitives{

   using point2d = quan::two_d::vect<double>;
   using point3d = quan::three_d::vect<double>;

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

   struct square_params_t{
      point2d size = point2d{1,1};
      bool center = false;
   };

   struct circle_params_t{
      double r = 1 ;
      facet_params_t fp;
   };

   struct sphere_params_t{
      double r = 1 ;
      facet_params_t fp;
   };

   struct cube_params_t {
      point3d size = point3d{1,1,1};
      bool center = false;
   };

   struct cylinder_params_t{
      double r1 = 1;
      double r2 = 1;
      double  h = 1;
      facet_params_t fp;
      bool center = false;
   };

   struct polyhedron_params_t {
      std::vector<primitives::point3d> points;
      std::vector<std::vector<size_t>> faces;
      int convexity = 1;
   };

   struct polygon_params_t{
      std::vector<primitives::point2d> points;
      std::vector<std::vector<size_t>> paths;
      int convexity = 1;
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
