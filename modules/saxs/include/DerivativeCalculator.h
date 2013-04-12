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
#include "ProfileFitter.h"
#include "Distribution.h"

IMPSAXS_BEGIN_NAMESPACE

//class ProfileFitter<ChiScore>;

class IMPSAXSEXPORT DerivativeCalculator : public base::RefCounted {
public:
  DerivativeCalculator(const Profile& exp_profile);

  /* compute the model-specific part of the derivative of the chi square
  * e.g. -2 * c * w_tilda(q) * (Iexp(q)-c*Icalc(q) + o)
  * for each q
  */
  std::vector<double> compute_gaussian_effect_size(const Profile& model_profile,
                                              const ProfileFitter<ChiScore>* pf,
                                              bool use_offset = false) const;

  /*
  * loop over all particles and rigid bodies, and call
  * compute_chisquare_derivative on them
  */
  void compute_all_derivatives(const Particles& particles,
       const std::vector<Particles>& rigid_bodies,
       const std::vector<core::RigidBody>& rigid_bodies_decorators,
       const Profile& model_profile, const std::vector<double>& effect_size,
       DerivativeAccumulator *acc) const;

protected:
  /*
  * precompute sinc_cos function and derivative of distance distribution
  */
  DeltaDistributionFunction precompute_derivative_helpers(const Profile&
          resampled_model_profile, const Particles& particles1,
          const Particles& particles2, std::vector<Floats>& sinc_cos_values)
      const ;

  /* compute dI(q)/dx_k for given q and k
  * dI(q)/dx_k = - 2 E^2(q) \sum_l (x_k-x_l)/d_{kl}^2 f_l f_k (sinc(q*d_{kl}) -
  * * cos(q*d_{kl}))
  */
  void compute_intensity_derivatives(const DeltaDistributionFunction&
          delta_dist, const std::vector<Floats>& sinc_cos_values,
          unsigned int iq, algebra::Vector3D &dIdx) const;

  //! compute derivatives for particles1 with respect to particles2
  /**
     This method is needed for rigid bodies when particles1 and particles2
     are particles of two rigid bodies. In this case the particles of the
     same rigid body are ignored.
     \param[in] model_profile The current profile of particles
     \param[in] particles1 Derivative will be computed for each particle
     \param[in] particles2 Derivative will be computed relative to this set
     \param[in] derivatives Output vector
     \return chi square score for current particle setup
  */
  void compute_chisquare_derivative(const Profile& model_profile,
                              const Particles& particles1,
                              const Particles& particles2,
                              std::vector<algebra::Vector3D >& derivatives,
                              const std::vector<double>& effect_size) const;
  void compute_chisquare_derivative(const Profile& model_profile,
                              const Particles& particles,
                              std::vector<algebra::Vector3D >& derivatives,
                              const std::vector<double>& effect_size) const
    {
        return compute_chisquare_derivative(model_profile, particles, particles,
                                  derivatives, effect_size);
    }

protected:
  const Profile exp_profile_;   //  experimental saxs profile

private:
  void compute_sinc_cos(Float pr_resolution, Float max_distance,
                        const Profile& model_profile,
                        std::vector<Floats>& output_values) const;

  void compute_profile_difference(const Profile& model_profile,
                                  const Float c, const Float offset,
                                  std::vector<double>& profile_diff) const;


};

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_DERIVATIVE_CALCULATOR_H */
