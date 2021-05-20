/**
 *  \file IMP/saxs/Profile.h
 *  \brief A class for profile storing and computation
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSAXS_PROFILE_H
#define IMPSAXS_PROFILE_H

#include <IMP/saxs/saxs_config.h>
#include <IMP/Object.h>
#include <IMP/warning_macros.h>

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
class IMPSAXSEXPORT Profile : public Object {
 public:
  // Constructors

  //! init from file
  /**
     \param[in] file_name profile file name
     \param[in] fit_file if true, intensities are read from column 3
     \param[in] max_q read till maximal q value = max_q, or all if max_q<=0
     \param[in] units gets 1, 2, or 3 for unknown q units, 1/A, or 1/nm
  */
  Profile(const std::string& file_name, bool fit_file = false, double max_q = 0.0, int units = 1);

  //! init for theoretical profile
  Profile(double qmin = 0.0, double qmax = 0.5, double delta = 0.005);

  // Various ways to compute a profile

  //! computes theoretical profile
  void calculate_profile(const Particles& particles,
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
  void calculate_profile_partial(const Particles& particles,
                                 const Vector<double>& surface = Vector<double>(),
                                 FormFactorType ff_type = HEAVY_ATOMS);

  //! compute profile for fitting with hydration layer and excluded volume
  void calculate_profile_partial(const Particles& particles1,
                                 const Particles& particles2,
                                 const Vector<double>& surface1 = Vector<double>(),
                                 const Vector<double>& surface2 = Vector<double>(),
                                 FormFactorType ff_type = HEAVY_ATOMS);

  void calculate_profile_reciprocal_partial(const Particles& particles,
                                 const Vector<double>& surface = Vector<double>(),
                                 FormFactorType ff_type = HEAVY_ATOMS);


  //! computes theoretical profile contribution from inter-molecular
  //! interactions between the particles
  void calculate_profile(const Particles& particles1,
                         const Particles& particles2,
                         FormFactorType ff_type = HEAVY_ATOMS) {
    calculate_profile_real(particles1, particles2, ff_type);
  }

  //! calculate Intensity at zero (= squared number of electrons)
  double calculate_I0(const Particles& particles,
                      FormFactorType ff_type = HEAVY_ATOMS);

  //! calculate profile for any type of Particles that have coordinates
  void calculate_profile_constant_form_factor(const Particles& particles,
                                              double form_factor = 1.0);


  // computes theoretical profile faster for cyclically symmetric particles
  // assumes that the units particles are ordered one after another in the
  // input particles vector (n - symmetry order)
  void calculate_profile_symmetric(const Particles& particles,
                                   unsigned int n,
                                   FormFactorType ff_type = HEAVY_ATOMS);

  //! convert to real space P(r) function P(r) = 1/2PI^2 Sum(I(q)*qr*sin(qr))
  void profile_2_distribution(RadialDistributionFunction& rd,
                              double max_distance) const;

  //! convert to reciprocal space I(q) = Sum(P(r)*sin(qr)/qr)
  void distribution_2_profile(const RadialDistributionFunction& r_dist);

  //! return a profile that is sampled on the q values of the exp_profile
  void resample(const Profile* exp_profile, Profile* resampled_profile) const;

  //! downsample the profile to a given number of points
  void downsample(Profile* downsampled_profile,
                  unsigned int point_number) const;

  //! compute radius of gyration with Guinier approximation
  /** ln[I(q)]=ln[I(0)] - (q^2*rg^2)/3
     \param[in] end_q_rg determines the range of profile used for approximation:
      i.e. q*rg < end_q_rg. Use 1.3 for globular proteins, 0.8 for elongated
  */
  double radius_of_gyration(double end_q_rg = 1.3) const;

  //! calculate mean intensity
  double mean_intensity() const;


  //! reads SAXS profile from file
  /**
     \param[in] file_name profile file name
     \param[in] fit_file if true, intensities are read from column 3
     \param[in] max_q read till maximal q value = max_q, or all if max_q<=0
     \param[in] units gets 1, 2, or 3 for unknown q units, 1/A, or 1/nm
   */
  void read_SAXS_file(const std::string& file_name, bool fit_file = false, double max_q = 0.0, int units = 1);

  //! print to file
  /** \param[in] file_name output file name
      \param[in] max_q output till maximal q value = max_q, or all if max_q<=0
  */
  void write_SAXS_file(const std::string& file_name, double max_q = 0.0) const;

  //! read a partial profile from file (7 columns)
  void read_partial_profiles(const std::string& file_name);

  //! write a partial profile to file
  void write_partial_profiles(const std::string& file_name) const;

  // Access functions

  //! return sampling resolution
  double get_delta_q() const { return delta_q_; }

  //! return minimal sampling point
  double get_min_q() const { return min_q_; }

  //! return maximal sampling point
  double get_max_q() const { return max_q_; }

  double get_intensity(unsigned int i) const { return intensity_(i); }
  double get_q(unsigned int i) const { return q_(i); }
  double get_error(unsigned int i) const { return error_(i); }
  double get_weight(unsigned int i) const {
    IMP_UNUSED(i);
    return 1.0;
  }

  const Eigen::VectorXf& get_qs() const { return q_; }
  const Eigen::VectorXf& get_intensities() const { return intensity_; }
  const Eigen::VectorXf& get_errors() const { return error_; }

  double get_average_radius() const { return average_radius_; }

  //! return number of entries in SAXS profile
  unsigned int size() const { return q_.size(); }

  //! checks the sampling of experimental profile
  bool is_uniform_sampling() const;

  bool is_partial_profile() const { return (partial_profiles_.size()>0); }

  std::string get_name() const { return name_; }

  unsigned int get_id() const { return id_; }

  // Modifiers
  void set_qs(const Eigen::VectorXf& q) { q_ = q; }
  void set_intensities(const Eigen::VectorXf& i) { intensity_ = i; }
  void set_errors(const Eigen::VectorXf& e) { error_ = e; }

  void set_intensity(unsigned int i, double iq) { intensity_(i) = iq; }

  //! required for reciprocal space calculation
  void set_ff_table(FormFactorTable* ff_table) { ff_table_ = ff_table; }

  void set_average_radius(double r) { average_radius_ = r; }

  void set_average_volume(double v) { average_volume_ = v; }

  void set_name(std::string name) { name_ = name; }

  void set_id(unsigned int id) { id_ = id; }

  void set_beam_profile(std::string beam_profile_file) {
    beam_profile_ = new Profile(beam_profile_file);
  }

  //! add simulated error
  void add_errors();

  //! add simulated noise
  void add_noise(double percentage = 0.03);

  //! computes full profile for given fitting parameters
  void sum_partial_profiles(double c1, double c2, bool check_cashed = true);

  //! add another profile - useful for rigid bodies
  void add(const Profile* other_profile, double weight = 1.0);

  //! add partial profiles
  void add_partial_profiles(const Profile* other_profile, double weight = 1.0);

  //! add other profiles - useful for weighted ensembles
  void add(const Vector<Profile*>& profiles,
           const Vector<double>& weights = Vector<double>());

  //! add other partial profiles
  void add_partial_profiles(const Vector<Profile*>& profiles,
                            const Vector<double>& weights = Vector<double>());

  //! background adjustment option
  void background_adjust(double start_q);

  //! scale
  void scale(double c);

  //! offset profile by c, I(q) = I(q) - c
  void offset(double c);

  // copy error bars from the matching experimental profile
  void copy_errors(const Profile* exp_profile);

  // parameter for E^2(q), used in faster calculation
  static const double modulation_function_parameter_;

  IMP_OBJECT_METHODS(Profile);

 protected:
  void init(unsigned int size = 0, unsigned int partial_profiles_size = 0);

 private:
  void calculate_profile_reciprocal(const Particles& particles,
                                    FormFactorType ff_type = HEAVY_ATOMS);

  void calculate_profile_reciprocal(const Particles& particles1,
                                    const Particles& particles2,
                                    FormFactorType ff_type = HEAVY_ATOMS);

  void calculate_profile_real(const Particles& particles,
                              FormFactorType ff_type = HEAVY_ATOMS);

  void calculate_profile_real(const Particles& particles1,
                              const Particles& particles2,
                              FormFactorType ff_type = HEAVY_ATOMS);

  void squared_distribution_2_profile(const RadialDistributionFunction& r_dist);

  void squared_distributions_2_partial_profiles(
      const Vector<RadialDistributionFunction>& r_dist);

  double radius_of_gyration_fixed_q(double end_q) const;

  double find_max_q(const std::string& file_name) const;

 protected:
  Eigen::VectorXf q_;  // q sampling points
  Eigen::VectorXf intensity_;
  Eigen::VectorXf error_;  // error bar of each point

  double min_q_, max_q_;        // minimal and maximal q values in the profile
  double delta_q_;              // profile sampling resolution
  FormFactorTable* ff_table_;  // pointer to form factors table

  // stores the intensity split into 6 for c1/c2 enumeration
  std::vector<Eigen::VectorXf> partial_profiles_;
  double c1_, c2_;

  bool experimental_;     // experimental profile read from file
  double average_radius_;  // average radius of the particles
  double average_volume_;  // average volume

  // mapping from q values to vector index for fast profile resampling
  mutable std::map<double, unsigned int> q_mapping_;

  std::string name_;  // file name
  unsigned int id_;   // identifier

  Profile* beam_profile_;
};

IMP_OBJECTS(Profile, Profiles);

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_PROFILE_H */
