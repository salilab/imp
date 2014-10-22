/**
 *  \file IMP/saxs/Profile.h
 *  \brief A class for profile storing and computation
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSAXS_PROFILE_H
#define IMPSAXS_PROFILE_H

#include <IMP/saxs/saxs_config.h>
#include <IMP/base/Object.h>
#include <IMP/base/warning_macros.h>

#include "FormFactorTable.h"
#include "Distribution.h"

#include <iostream>
#include <vector>

IMPSAXS_BEGIN_NAMESPACE

class RadialDistributionFunction;

/**
   Basic profile class, can be initialized from the input file
   (experimental or theoretical) or computed from a set of kernel::Model
   kernel::Particles (theoretical)
*/
class IMPSAXSEXPORT Profile : public base::Object {
 public:
  // Constructors

  //! init from file
  Profile(const String& file_name, bool fit_file = false);

  //! init for theoretical profile
  Profile(Float qmin = 0.0, Float qmax = 0.5, Float delta = 0.005);

  // Various ways to compute a profile

  //! computes theoretical profile
  void calculate_profile(const kernel::Particles& particles,
                         FormFactorType ff_type = HEAVY_ATOMS,
                         bool reciprocal = false) {
    if (!reciprocal)
      calculate_profile_real(particles, ff_type);
    else
      calculate_profile_reciprocal(particles, ff_type);
  }

  //! compute profile for fitting with hydration layer and excluded volume
  /**
    A partial profile is a pre-computed profile, where intensities are
    split into 6 parts that can be summed up into a regular profile
    given a pair of c1/c2 values by sum_partial_profiles function.
    see FoXS paper for details.
  */
  void calculate_profile_partial(const kernel::Particles& particles,
                                 const Floats& surface = Floats(),
                                 FormFactorType ff_type = HEAVY_ATOMS);

  //! compute profile for fitting with hydration layer and excluded volume
  void calculate_profile_partial(const kernel::Particles& particles1,
                                 const kernel::Particles& particles2,
                                 const Floats& surface1 = Floats(),
                                 const Floats& surface2 = Floats(),
                                 FormFactorType ff_type = HEAVY_ATOMS);

  void calculate_profile_reciprocal_partial(const kernel::Particles& particles,
                                            const Floats& surface = Floats(),
                                            FormFactorType ff_type =
                                                HEAVY_ATOMS);

  //! computes theoretical profile contribution from inter-molecular
  //! interactions between the particles
  void calculate_profile(const kernel::Particles& particles1,
                         const kernel::Particles& particles2,
                         FormFactorType ff_type = HEAVY_ATOMS) {
    calculate_profile_real(particles1, particles2, ff_type);
  }

  //! calculate Intensity at zero (= squared number of electrons)
  Float calculate_I0(const kernel::Particles& particles,
                     FormFactorType ff_type = HEAVY_ATOMS);

  //! calculate profile for any type of kernel::Particles that have coordinates
  void calculate_profile_constant_form_factor(
      const kernel::Particles& particles, Float form_factor = 1.0);

  // computes theoretical profile faster for cyclically symmetric particles
  // assumes that the units particles are ordered one after another in the
  // input particles vector (n - symmetry order)
  void calculate_profile_symmetric(const kernel::Particles& particles,
                                   unsigned int n,
                                   FormFactorType ff_type = HEAVY_ATOMS);

  //! convert to real space P(r) function P(r) = 1/2PI^2 Sum(I(q)*qr*sin(qr))
  void profile_2_distribution(RadialDistributionFunction& rd,
                              Float max_distance) const;

  //! convert to reciprocal space I(q) = Sum(P(r)*sin(qr)/qr)
  void distribution_2_profile(const RadialDistributionFunction& r_dist);

  //! return a profile that is sampled on the q values of the exp_profile
  void resample(const Profile* exp_profile, Profile* resampled_profile,
                bool partial_profiles = false) const;

  //! downsample the profile to a given number of points
  void downsample(Profile* downsampled_profile,
                  unsigned int point_number) const;

  //! compute radius of gyration with Guinier approximation
  /** ln[I(q)]=ln[I(0)] - (q^2*rg^2)/3
     \param[in] end_q_rg determines the range of profile used for approximation:
      i.e. q*rg < end_q_rg. Use 1.3 for globular proteins, 0.8 for elongated
  */
  double radius_of_gyration(double end_q_rg = 1.3) const;

  // IO functions

  //! reads SAXS profile from file
  /**
     \param[in] file_name profile file name
     \param[in] fit_file if true, intensities are read from column 3
   */
  void read_SAXS_file(const String& file_name, bool fit_file = false);

  //! print to file
  /** \param[in] file_name output file name
      \param[in] max_q output till maximal q value = max_q, or all if max_q<=0
  */
  void write_SAXS_file(const String& file_name, Float max_q = 0.0) const;

  //! read a partial profile from file (7 columns)
  void read_partial_profiles(const String& file_name);

  //! write a partial profile to file
  void write_partial_profiles(const String& file_name) const;

  // Access functions

  //! return sampling resolution
  Float get_delta_q() const { return delta_q_; }

  //! return minimal sampling point
  Float get_min_q() const { return min_q_; }

  //! return maximal sampling point
  Float get_max_q() const { return max_q_; }

  Float get_intensity(unsigned int i) const { return intensity_[i]; }
  Float get_q(unsigned int i) const { return q_[i]; }
  Float get_error(unsigned int i) const { return error_[i]; }
  Float get_weight(unsigned int i) const {
    IMP_UNUSED(i);
    return 1.0;
  }
  Float get_average_radius() const { return average_radius_; }

  //! return number of entries in SAXS profile
  unsigned int size() const { return q_.size(); }

  //! checks the sampling of experimental profile
  bool is_uniform_sampling() const;

  std::string get_name() const { return name_; }

  unsigned int get_id() const { return id_; }

  // Modifiers

  void set_intensity(unsigned int i, Float iq) { intensity_[i] = iq; }

  //! required for reciprocal space calculation
  void set_ff_table(FormFactorTable* ff_table) { ff_table_ = ff_table; }

  void set_average_radius(Float r) { average_radius_ = r; }

  void set_average_volume(Float v) { average_volume_ = v; }

  void set_name(std::string name) { name_ = name; }

  void set_id(unsigned int id) { id_ = id; }

  void set_beam_profile(std::string beam_profile_file) {
    beam_profile_ = new Profile(beam_profile_file);
  }

  //! add intensity entry to profile
  void add_entry(Float q, Float intensity, Float error = 1.0) {
    q_.push_back(q);
    intensity_.push_back(intensity);
    error_.push_back(error);
  }

  //! add simulated error
  void add_errors();

  //! add simulated noise
  void add_noise(Float percentage = 0.03);

  //! computes full profile for given fitting parameters
  void sum_partial_profiles(Float c1, Float c2, bool check_cashed = true);

  //! add another profile - useful for rigid bodies
  void add(const Profile* other_profile, Float weight = 1.0);

  //! add partial profiles
  void add_partial_profiles(const Profile* other_profile, Float weight = 1.0);

  //! add other profiles - useful for weighted ensembles
  void add(const std::vector<Profile*>& profiles,
           const std::vector<Float>& weights = std::vector<Float>());

  //! add other partial profiles
  void add_partial_profiles(const std::vector<Profile*>& profiles,
                            const std::vector<Float>& weights =
                                std::vector<Float>());

  //! background adjustment option
  void background_adjust(double start_q);

  //! scale
  void scale(Float c);

  //! offset profile by c, I(q) = I(q) - c
  void offset(Float c);

  // copy error bars from the matching experimental profile
  void copy_errors(const Profile* exp_profile);

  // parameter for E^2(q), used in faster calculation
  static const Float modulation_function_parameter_;

  IMP_OBJECT_METHODS(Profile);

 protected:
  void init();

 private:
  void calculate_profile_reciprocal(const kernel::Particles& particles,
                                    FormFactorType ff_type = HEAVY_ATOMS);

  void calculate_profile_reciprocal(const kernel::Particles& particles1,
                                    const kernel::Particles& particles2,
                                    FormFactorType ff_type = HEAVY_ATOMS);

  void calculate_profile_real(const kernel::Particles& particles,
                              FormFactorType ff_type = HEAVY_ATOMS);

  void calculate_profile_real(const kernel::Particles& particles1,
                              const kernel::Particles& particles2,
                              FormFactorType ff_type = HEAVY_ATOMS);

  void squared_distribution_2_profile(const RadialDistributionFunction& r_dist);

  void squared_distributions_2_partial_profiles(
      const std::vector<RadialDistributionFunction>& r_dist);

  double radius_of_gyration_fixed_q(double end_q) const;

 protected:
  std::vector<double> q_;  // q sampling points
  std::vector<double> intensity_;
  std::vector<double> error_;  // error bar of each point

  Float min_q_, max_q_;        // minimal and maximal s values  in the profile
  Float delta_q_;              // profile sampling resolution
  FormFactorTable* ff_table_;  // pointer to form factors table

  // stores the intensity split into 6 for c1/c2 enumeration
  std::vector<std::vector<double> > partial_profiles_;
  Float c1_, c2_;

  bool experimental_;     // experimental profile read from file
  Float average_radius_;  // average radius of the particles
  Float average_volume_;  // average volume

  // mapping from q values to vector index for fast profile resampling
  std::map<float, unsigned int> q_mapping_;

  std::string name_;  // file name
  unsigned int id_;   // identifier

  Profile* beam_profile_;
};

IMP_OBJECTS(Profile, Profiles);

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_PROFILE_H */
