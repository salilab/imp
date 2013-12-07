/**
 * \file IMP/saxs/DerivativeCalculator.h
 * \brief A class for computing SAXS derivatives
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSAXS_DERIVATIVE_CALCULATOR_H
#define IMPSAXS_DERIVATIVE_CALCULATOR_H

#include "Profile.h"
#include "Distribution.h"
#include <IMP/base/Object.h>

IMPSAXS_BEGIN_NAMESPACE

/**
   A class for computing derivatives with respect to SAXS
   Note: the calculation is expensive, make sure it helps if you use it!
*/
class IMPSAXSEXPORT DerivativeCalculator : public base::Object {
 public:
  DerivativeCalculator(const Profile* exp_profile);

  //! compute derivatives for particles1 with respect to particles2
  /**
     This method is needed for rigid bodies when particles1 and particles2
     are particles of two rigid bodies. In this case the particles of the
     same rigid body are ignored.
     \param[in] model_profile The current profile of particles
     \param[in] particles1 Derivative will be computed for each particle
     \param[in] particles2 Derivative will be computed relative to this set
     \param[in] derivatives Output vector
     \param[in] effect_size Effect size
  */
  void compute_chisquare_derivative(
      const Profile* model_profile, const kernel::Particles& particles1,
      const kernel::Particles& particles2,
      std::vector<algebra::Vector3D>& derivatives,
      const std::vector<double>& effect_size) const;

  //! compute derivatives for particles
  /**
     \param[in] model_profile The current profile of particles
     \param[in] particles Derivative will be computed for each particle
     \param[in] derivatives Output vector
     \param[in] effect_size Effect size
  */
  void compute_chisquare_derivative(
      const Profile* model_profile, const kernel::Particles& particles,
      std::vector<algebra::Vector3D>& derivatives,
      const std::vector<double>& effect_size) const {
    return compute_chisquare_derivative(model_profile, particles, particles,
                                        derivatives, effect_size);
  }

  void compute_gaussian_effect_size(const Profile* model_profile, const Float c,
                                    const Float offset,
                                    std::vector<double>& effect_size) const;

 protected:
  /*
  * precompute sinc_cos function and derivative of distance distribution
  */
  DeltaDistributionFunction precompute_derivative_helpers(
      const Profile* resampled_model_profile,
      const kernel::Particles& particles1, const kernel::Particles& particles2,
      std::vector<Floats>& sinc_cos_values) const;

  /* compute dI(q)/dx_k for given q and k
  * dI(q)/dx_k = - 2 E^2(q) \sum_l (x_k-x_l)/d_{kl}^2 f_l f_k (sinc(q*d_{kl}) -
  * * cos(q*d_{kl}))
  */
  void compute_intensity_derivatives(
      const DeltaDistributionFunction& delta_dist,
      const std::vector<Floats>& sinc_cos_values, unsigned int iq,
      algebra::Vector3D& dIdx) const;

 protected:
  const Profile* exp_profile_;  //  experimental saxs profile

 private:
  void compute_sinc_cos(Float pr_resolution, Float max_distance,
                        const Profile* model_profile,
                        std::vector<Floats>& output_values) const;
};

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_DERIVATIVE_CALCULATOR_H */
