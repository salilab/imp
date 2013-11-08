/**
 *  \file VariancedProfile.cpp
 *  \brief A class for profile storing and computation
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/saxs/VariancedProfile.h>
#include <IMP/saxs/Distribution.h>
#include <IMP/saxs/utility.h>
#include <IMP/saxs/internal/sinc_function.h>
#include <IMP/saxs/internal/exp_function.h>
#include <IMP/saxs/internal/variance_helpers.h>

#include <IMP/base/math.h>
#include <IMP/core/XYZ.h>
#include <IMP/algebra/utility.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/ParabolicFit.h>
#include <IMP/algebra/LinearFit.h>
#include <IMP/constants.h>
#include <IMP/random.h>

#include <boost/algorithm/string.hpp>
#include <boost/random.hpp>
#include <boost/math/special_functions/sinc.hpp>
#include <boost/random/normal_distribution.hpp>

#include <fstream>
#include <string>

#define IMP_SAXS_DELTA_LIMIT  1.0e-15

IMPSAXS_BEGIN_NAMESPACE

const Float VariancedProfile::modulation_function_parameter_ = 0.23;

VariancedProfile::VariancedProfile(Float qmin, Float qmax, Float delta):
  Profile(qmin, qmax, delta) {}

void VariancedProfile::init_variances() {
  variances_.clear();
  int number_of_q_entries = (int)std::ceil((max_q_ - min_q_) / delta_q_ )+1;
  for(int i=0; i<number_of_q_entries; i++) {
      variances_.push_back(std::vector<double>((number_of_q_entries-i), 0.0));
  }
}

void VariancedProfile::calculate_profile_real(
                                     const kernel::Particles& particles,
                                     FormFactorType ff_type,
                                     double variance_tau) {
  IMP_LOG_TERSE("start real profile calculation for "
                 << particles.size() << " particles" << std::endl);
  RadialDistributionFunction r_dist; //fi(0) fj(0)
  RadialDistributionFunction r_dist2; //fi(0)^2 fj(0)^2
  // prepare coordinates and form factors in advance, for faster access
  std::vector<algebra::Vector3D> coordinates;
  get_coordinates(particles, coordinates);
  Floats form_factors;
  get_form_factors(particles, ff_table_, form_factors, ff_type);

  // iterate over pairs of atoms
  for (unsigned int i = 0; i < coordinates.size(); i++) {
    for (unsigned int j = i + 1; j < coordinates.size(); j++) {
      Float dist = get_squared_distance(coordinates[i], coordinates[j]);
      double prod = form_factors[i] * form_factors[j];
      r_dist.add_to_distribution(dist, 2 * prod);
      r_dist2.add_to_distribution(dist, 2 * prod * prod);
    }
    // add autocorrelation part
    r_dist.add_to_distribution(0.0, square(form_factors[i]));
    r_dist2.add_to_distribution(0.0, square(square(form_factors[i])));
  }
  squared_distribution_2_profile(r_dist, r_dist2, variance_tau);
}

void VariancedProfile::calculate_profile_real(
                                     const kernel::Particles& particles1,
                                     const kernel::Particles& particles2,
                                     FormFactorType ff_type,
                                     double variance_tau) {
  IMP_LOG_TERSE( "start real profile calculation for "
          << particles1.size() << " + " << particles2.size()
          << " particles" << std::endl);
  RadialDistributionFunction r_dist; //fi(0) fj(0)
  RadialDistributionFunction r_dist2; //fi(0)^2 fj(0)^2

  // copy coordinates and form factors in advance, to avoid n^2 copy operations
  std::vector<algebra::Vector3D> coordinates1, coordinates2;
  get_coordinates(particles1, coordinates1);
  get_coordinates(particles2, coordinates2);
  Floats form_factors1, form_factors2;
  get_form_factors(particles1, ff_table_, form_factors1, ff_type);
  get_form_factors(particles2, ff_table_, form_factors2, ff_type);

  // iterate over pairs of atoms
  for (unsigned int i = 0; i < coordinates1.size(); i++) {
    for (unsigned int j = 0; j < coordinates2.size(); j++) {
      Float dist = get_squared_distance(coordinates1[i], coordinates2[j]);
      double prod = form_factors1[i] * form_factors2[j];
      r_dist.add_to_distribution(dist, 2 * prod);
      r_dist2.add_to_distribution(dist, 2*prod*prod);
    }
  }
  squared_distribution_2_profile(r_dist, r_dist2, variance_tau);
}

void VariancedProfile::
squared_distribution_2_profile(const RadialDistributionFunction& r_dist,
                               const RadialDistributionFunction& r_dist2,
                               double variance_tau) {
  squared_distribution_2_profile(r_dist);
  init_variances();

  // precompute square roots of distances
  std::vector<float> distances(r_dist.size(), 0.0);
  for (unsigned int r = 0; r < r_dist.size(); r++)
    if(r_dist[r] != 0.0)  distances[r] = sqrt(r_dist.index2dist(r));

  for (unsigned int k = 0; k < size(); k++) {
    intensity_[k] *= std::exp(-0.5*square(variance_tau*q_[k]));
  }

  // iterate over rows
  for (unsigned int i = 0; i < size(); i++) {
   // iterate over columns
   for (unsigned int j = i; j < size(); j++) {
     double q1 = q_[i];
     double q2 = q_[j];
      // iterate over radial distribution
     double contrib=0;
     if (q1*q2 != 0){ //else contrib is zero
      for (unsigned int r = 0; r < r_dist.size(); r++) {
        if(r_dist[r] == 0.0) continue;
        double dist = distances[r];
        if(dist == 0.0) continue;
        const double sqrt2 = 1.4142135623730950488;
        double a = q1*variance_tau/sqrt2;
        double b = q2*variance_tau/sqrt2;
        double c = dist/(variance_tau*sqrt2);
         //exponent beats erf at high distances, so assume infs and nans mean 0
        double A(internal::A(a,b,c));
        contrib += A*r_dist2[r];
      }
     }
    double var = contrib
                *std::exp(- modulation_function_parameter_*(q1*q1+q2*q2));
    variances_[i][j-i] = var;
   }
  }
}

IMPSAXS_END_NAMESPACE
