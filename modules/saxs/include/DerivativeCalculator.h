/**
 * \file DerivativeCalculator.h \brief A class for computing SAXS derivatives
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPSAXS_DERIVATIVE_CALCULATOR_H
#define IMPSAXS_DERIVATIVE_CALCULATOR_H

#include "Profile.h"

IMPSAXS_BEGIN_NAMESPACE

class IMPSAXSEXPORT DerivativeCalculator : public RefCounted {
public:
  DerivativeCalculator(const Profile& exp_profile);

  //! compute derivatives for particles1 with respect to particles2
  /**
     This method is needed for rigid bodies when particles1 and particles2
     are particles of two rigid bodies. In this case the particles of the
     same rigid body are ignored.
     \param[in] model_profile The current profile of particles
     \param[in] particles1 Derivative will be computed for each particle
     \param[in] particles2 Derivative will be computed relative to this set
     \param[in] derivatives Output vector
     \param[in] use_offset if true, non zero offset for fitting is used
     \return chi square score for current particle setup
  */
  void compute_chi_derivative(const Profile& model_profile,
                              const Particles& particles1,
                              const Particles& particles2,
                              std::vector<algebra::Vector3D >& derivatives,
                              bool use_offset = false) const;
  void compute_chi_derivative(const Profile& model_profile,
                              const Particles& particles,
                              std::vector<algebra::Vector3D >& derivatives,
                              bool use_offset = false) const {
    return compute_chi_derivative(model_profile, particles, particles,
                                  derivatives, use_offset);
  }

protected:
    // helper function for derivatives
  void compute_profile_difference(const Profile& model_profile,
                                  const Float c, const Float offset,
                                  Floats& profile_diff) const;

  // helper function for derivatives
  void compute_sinc_cos(Float pr_resolution, Float max_distance,
                        const Profile& model_profile,
                        std::vector<Floats>& output_values) const;

protected:
  const Profile exp_profile_;   //  experimental saxs profile
};

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_DERIVATIVE_CALCULATOR_H */
