/**
 *  \file saxs::Restraint.h
 *  \brief Calculate score based on fit to SAXS profile.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/saxs/Restraint.h>
#include <IMP/base/log.h>

#include <IMP/atom/Hierarchy.h>
#include <IMP/core/LeavesRefiner.h>

IMPSAXS_BEGIN_NAMESPACE

Restraint::Restraint(const kernel::Particles& particles,
                     const Profile* exp_profile, FormFactorType ff_type)
    : IMP::Restraint(IMP::internal::get_model(particles), "SAXS restraint") {

  handler_ = new RigidBodiesProfileHandler(particles, ff_type);
  profile_fitter_ = new ProfileFitter<ChiScore>(exp_profile);
  derivative_calculator_ = new DerivativeCalculator(exp_profile);
}

ModelObjectsTemp Restraint::do_get_inputs() const {
  return handler_->do_get_inputs();
}

//! Calculate the score and the derivatives for particles of the restraint.
/** \param[in] acc If true (not nullptr), partial first derivatives should be
                          calculated.
    \return score associated with this restraint for the given state of
            the model.
*/
double Restraint::unprotected_evaluate(DerivativeAccumulator* acc) const {

  IMP_LOG_TERSE("SAXS kernel::Restraint::evaluate score\n");

  IMP_NEW(Profile, model_profile, ());
  handler_->compute_profile(model_profile);
  Float score = profile_fitter_->compute_score(model_profile);
  bool calc_deriv = acc ? true : false;
  if (!calc_deriv) return score;

  IMP_LOG_TERSE("SAXS kernel::Restraint::compute derivatives\n");

  // do we need to resample the curve since it's just been created??
  // yes, since it does not correspond to the experimental one
  IMP_NEW(Profile, resampled_profile, ());
  profile_fitter_->resample(model_profile, resampled_profile);

  std::vector<double> effect_size;  // gaussian model-specific derivative
                                    // weights
  Float offset = 0.0;
  Float c = profile_fitter_->compute_scale_factor(model_profile);
  derivative_calculator_->compute_gaussian_effect_size(model_profile, c, offset,
                                                       effect_size);

  handler_->compute_derivatives(derivative_calculator_, model_profile,
                                effect_size, acc);

  IMP_LOG_TERSE("SAXS kernel::Restraint::done derivatives, score " << score
                                                                   << "\n");
  return score;
}

IMPSAXS_END_NAMESPACE
