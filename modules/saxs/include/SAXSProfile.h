/**
 *  \file SAXSProfile.h   \brief A class for profile storing and computation
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */
#ifndef IMPSAXS_SAXS_PROFILE_H
#define IMPSAXS_SAXS_PROFILE_H

#include "config.h"
#include "FormFactorTable.h"
#include "RadialDistributionFunction.h"

#include <IMP/Model.h>

#include <iostream>
#include <vector>

IMPSAXS_BEGIN_NAMESPACE

class RadialDistributionFunction;

/* Basic profile class, can be initialized from the input file
   (experimental or theoretical) or computed from a set of Model
   Particles (theoretical)
*/
class IMPSAXSEXPORT SAXSProfile {
public:
  //! init from file
  SAXSProfile(const String& file_name);

  //! init for theoretical profile
  SAXSProfile(Float smin, Float smax, Float delta, FormFactorTable* ff_table);

private:
  class IntensityEntry {
  public:
    IntensityEntry() : s_(0.0), intensity_(0.0), error_(1.0) {}
    IntensityEntry(Float s) : s_(s), intensity_(0.0), error_(1.0) {}

    Float s_;
    Float intensity_;
    Float error_;
  };

  friend std::ostream& operator<<(std::ostream& s, const IntensityEntry& e);

  friend std::istream& operator>>(std::istream& s, IntensityEntry& e);

public:
  //! computes theoretical profile
  void calculate_profile(const std::vector<Particle*>& particles) {
    calculate_profile_real(particles);
  }

  //! computes theoretical profile contribution from iter-molecular
  // interactions between the particles
  void calculate_profile(const std::vector<Particle*>& particles1,
                         const std::vector<Particle*>& particles2) {
    calculate_profile_real(particles1, particles2);
  }

  //! reads SAXS profile from file
  void read_SAXS_file(const String& file_name);

  //! print to file
  void write_SAXS_file(const String& file_name);

  //! compute chi value (assumes the same sampling range!)
  Float compute_chi_score(const SAXSProfile& profile) const;

  //! return sampling resolution
  Float get_delta_s() const { return delta_s_; }

  //! return minimal sampling point
  Float get_min_s() const { return min_s_; }

  //! return maximal sampling point
  Float get_max_s() const { return max_s_; }

private:
  void init();

  void calculate_profile_reciprocal(const std::vector<Particle*>& particles);

  void calculate_profile_reciprocal(const std::vector<Particle*>& particles1,
                                    const std::vector<Particle*>& particles2);

  void calculate_profile_real(const std::vector<Particle*>& particles);

  void calculate_profile_real(const std::vector<Particle*>& particles1,
                              const std::vector<Particle*>& particles2);

  void radial_distribution_2_profile(const RadialDistributionFunction& r_dist);

  Float sinc(Float value) {
    if(fabs(value) < 0.0000000000000001) return 1.0;
    return sin(value)/value;
  }

  void write_SAXS_fit_file(const String& file_name,
                           const SAXSProfile& saxs_profile,
                           const Float c) const;

protected:
  std::vector<IntensityEntry> profile_; // the profile
  Float min_s_, max_s_; // minimal and maximal s values  in the profile
  Float delta_s_; // profile sampling resolution
  FormFactorTable* ff_table_; // pointer to form factors table
};

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_SAXS_PROFILE_H */
