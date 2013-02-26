/**
 *  \file IMP/saxs/Profile.h
 *  \brief A class for profile storing and computation
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSAXS_PROFILE_H
#define IMPSAXS_PROFILE_H

#include <IMP/saxs/saxs_config.h>
#include "FormFactorTable.h"
#include "Distribution.h"

#include <iostream>
#include <vector>

IMPSAXS_BEGIN_NAMESPACE

class RadialDistributionFunction;

/**
   Basic profile class, can be initialized from the input file
   (experimental or theoretical) or computed from a set of Model
   Particles (theoretical)
*/
class IMPSAXSEXPORT Profile {
public:
  //! init from file
  Profile(const String& file_name);

  //! init for theoretical profile
  Profile(Float qmin = 0.0, Float qmax = 0.5, Float delta = 0.005);

private:
  class IntensityEntry {
  public:
    IntensityEntry() : q_(0.0), intensity_(0.0), error_(1.0), weight_(1.0) {}
    IntensityEntry(Float q) : q_(q),intensity_(0.0),error_(1.0),weight_(1.0) {}
    IntensityEntry(Float q, Float intensity, Float error)
      : q_(q), intensity_(intensity), error_(error), weight_(1.0) {}

    Float q_;
    Float intensity_;
    Float error_;
    Float weight_;
  };

  friend std::ostream& operator<<(std::ostream& q, const IntensityEntry& e);

  friend std::istream& operator>>(std::istream& q, IntensityEntry& e);

public:
  //! computes theoretical profile
  void calculate_profile(const Particles& particles,
                         FormFactorType ff_type = HEAVY_ATOMS,
                         bool reciprocal=false,
                         bool variance=false,
                         double variance_tau=0.1) {
    IMP_USAGE_CHECK(!(reciprocal && variance),
            "variance not implemented in reciprocal calculation");
    if(!reciprocal) calculate_profile_real(particles, ff_type,
            variance, variance_tau);
    else calculate_profile_reciprocal(particles, ff_type);
  }

  //! compute profile for fitting with hydration layer and excluded volume
  void calculate_profile_partial(const Particles& particles,
                                 const Floats& surface = Floats(),
                                 FormFactorType ff_type = HEAVY_ATOMS);

  //! compute profile for fitting with hydration layer and excluded volume
  void calculate_profile_partial(const Particles& particles1,
                                 const Particles& particles2,
                                 const Floats& surface1 = Floats(),
                                 const Floats& surface2 = Floats(),
                                 FormFactorType ff_type = HEAVY_ATOMS);

  void calculate_profile_reciprocal_partial(const Particles& particles,
                                            const Floats& surface = Floats(),
                                          FormFactorType ff_type = HEAVY_ATOMS);

  //! computes theoretical profile contribution from iter-molecular
  //! interactions between the particles
  void calculate_profile(const Particles& particles1,
                         const Particles& particles2,
                         FormFactorType ff_type = HEAVY_ATOMS,
                         bool variance=false,
                         double variance_tau=0.1) {
    calculate_profile_real(particles1, particles2, ff_type,
            variance, variance_tau);
  }

  //! calculate Intensity at zero (= squared number of electrons)
  Float calculate_I0(const Particles& particles,
                     FormFactorType ff_type = HEAVY_ATOMS);

  //! calculate profile for any type of Particles that have coordinates
  void calculate_profile_constant_form_factor(const Particles& particles,
                                              Float form_factor = 1.0);

  // computes theoretical profile faster for cyclically symmetric particles
  // assumes that the units particles are ordered one after another in the
  // input particles vector (n - symmetry order)
  void calculate_profile_symmetric(const Particles& particles, unsigned int n,
                                   FormFactorType ff_type = HEAVY_ATOMS);

  //! convert to real space P(r) function P(r) = 1/2PI^2 Sum(I(q)*qr*sin(qr))
  void profile_2_distribution(RadialDistributionFunction& rd,
                              Float max_distance) const;

  //! convert to reciprocal space I(q) = Sum(P(r)*sin(qr)/qr)
  void distribution_2_profile(const RadialDistributionFunction& r_dist);

  //! add another profile - useful for rigid bodies
  void add(const Profile& other_profile, Float weight = 1.0);

  //! add partial profiles
  void add_partial_profiles(const Profile& other_profile, Float weight = 1.0);

  //! background adjustment option
  void background_adjust(double start_q);

  //! scale
  void scale(Float c);

  //! offset profile by c, I(q) = I(q) - c
  void offset(Float c);

  //! reads SAXS profile from file
  void read_SAXS_file(const String& file_name);

  //! print to file
  /** \param[in] file_name output file name
      \param[in] max_q output till maximal q value = max_q, or all if max_q<=0
  */
  void write_SAXS_file(const String& file_name, Float max_q=0.0) const;

  void write_partial_profiles(const String& file_name) const;

  // compute radius of gyration with Guinier approximation
  // ln[I(q)]=ln[I(0)] - (q^2*rg^2)/3
  // end_q_rg determines the range of profile used for approximation:
  // i.e. q*rg < end_q_rg. Use 1.3 for globular proteins, 0.8 for elongated
  double radius_of_gyration(double end_q_rg = 1.3) const;

  //! return sampling resolution
  Float get_delta_q() const { return delta_q_; }

  //! return minimal sampling point
  Float get_min_q() const { return min_q_; }

  //! return maximal sampling point
  Float get_max_q() const { return max_q_; }

  //! return number of entries in SAXS profile
  unsigned int size() const { return profile_.size(); }

  // Profile access functions
  Float get_intensity(unsigned int i) const { return profile_[i].intensity_; }
  Float get_q(unsigned int i) const { return profile_[i].q_; }
  Float get_error(unsigned int i) const { return profile_[i].error_; }
  Float get_weight(unsigned int i) const { return profile_[i].weight_; }
  Float get_variance(unsigned int i, unsigned int j) const
  { unsigned a=std::min(i,j); unsigned b=std::max(i,j);
      return variances_[a][b-a]; }

  Float get_average_radius() const { return average_radius_; }

  void set_intensity(unsigned int i, Float iq) { profile_[i].intensity_ = iq; }

  //! required for reciprocal space calculation
  void set_ff_table(FormFactorTable* ff_table) { ff_table_ = ff_table; }

  void set_average_radius(Float r) { average_radius_ = r; }

  void set_average_volume(Float v) { average_volume_ = v; }

  //! add intensity entry to profile
  void add_entry(Float q, Float intensity, Float error=1.0) {
    profile_.push_back(IntensityEntry(q, intensity, error));
  }

  //! checks the sampling of experimental profile
  bool is_uniform_sampling() const;

  //! add simulated error
  void add_errors();

  //! add simulated noise
  void add_noise(Float percentage = 0.03);

  //! computes full profile for given fitting parameters
  void sum_partial_profiles(Float c1, Float c2, Profile& out_profile);

  // parameter for E^2(q), used in faster calculation
  static const Float modulation_function_parameter_;

 private:
  void init(bool variance = false);

  void calculate_profile_reciprocal(const Particles& particles,
                                    FormFactorType ff_type = HEAVY_ATOMS);

  void calculate_profile_reciprocal(const Particles& particles1,
                                    const Particles& particles2,
                                    FormFactorType ff_type = HEAVY_ATOMS);

  void calculate_profile_real(const Particles& particles,
                              FormFactorType ff_type = HEAVY_ATOMS,
                              bool variance = false,
                              double variance_tau = 0.1);

  void calculate_profile_real(const Particles& particles1,
                              const Particles& particles2,
                              FormFactorType ff_type = HEAVY_ATOMS,
                              bool variance = false,
                              double variance_tau = 0.1);

  void squared_distribution_2_profile(const RadialDistributionFunction& r_dist,
          const RadialDistributionFunction& r_dist2,
          bool variance=false, double variance_tau=0.1);

  void squared_distributions_2_partial_profiles(
                         const std::vector<RadialDistributionFunction>& r_dist);

  double radius_of_gyration_fixed_q(double end_q) const;

 protected:
  std::vector<IntensityEntry> profile_; // the profile
  std::vector<std::vector<double> > variances_; //profile variances
  Float min_q_, max_q_; // minimal and maximal s values  in the profile
  Float delta_q_; // profile sampling resolution
  FormFactorTable* ff_table_; // pointer to form factors table
  std::vector<Profile> partial_profiles_;
  bool experimental_; // experimental profile read from file
  Float average_radius_; // average radius of the particles
  Float average_volume_; // average volume
};

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_PROFILE_H */
