/**
 *  \file IMP/saxs/VariancedProfile.h
 *  \brief A class for profile storing and computation
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSAXS_PROFILE_H
#define IMPSAXS_PROFILE_H

#include <IMP/saxs/saxs_config.h>
#include <IMP/base/Object.h>
#include <IMP/base/warning_macros.h>

#include "Profile.h"
#include "FormFactorTable.h"
#include "Distribution.h"

#include <iostream>
#include <vector>

IMPSAXS_BEGIN_NAMESPACE

class RadialDistributionFunction;

class IMPSAXSEXPORT VariancedProfile : public Profile {
public:
  // Constructors

  //! init for theoretical profile
  VariancedProfile(Float qmin = 0.0, Float qmax = 0.5, Float delta = 0.005);

  IMP_OBJECT_METHODS(Profile);

  //! computes theoretical profile
  void calculate_profile(const kernel::Particles& particles,
                         FormFactorType ff_type = HEAVY_ATOMS,
                         double variance_tau=0.1) {
    calculate_profile_real(particles, ff_type, variance_tau);
  }

  //! computes theoretical profile contribution from iter-molecular
  //! interactions between the particles
  void calculate_profile(const kernel::Particles& particles1,
                         const kernel::Particles& particles2,
                         FormFactorType ff_type = HEAVY_ATOMS,
                         double variance_tau=0.1) {
    calculate_profile_real(particles1, particles2, ff_type,
            variance_tau);
  }

  Float get_variance(unsigned int i, unsigned int j) const
  { unsigned a=std::min(i,j); unsigned b=std::max(i,j);
      return variances_[a][b-a]; }

 protected:
  void init_variances();

  void calculate_profile_real(const kernel::Particles& particles,
                              FormFactorType ff_type = HEAVY_ATOMS,
                              double variance_tau = 0.1);

  void calculate_profile_real(const kernel::Particles& particles1,
                              const kernel::Particles& particles2,
                              FormFactorType ff_type = HEAVY_ATOMS,
                              double variance_tau = 0.1);

  void squared_distribution_2_profile(const RadialDistributionFunction& r_dist,
                                      const RadialDistributionFunction& r_dist2,
                                      double variance_tau=0.1);

 protected:
  std::vector<std::vector<double> > variances_; //profile variances

};

IMP_OBJECTS(VariancedProfile, VariancedProfiles);

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_PROFILE_H */
