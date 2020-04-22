/**
 *  \file saxs::Restraint.h
 *  \brief Calculate score based on fit to SAXS profile.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/saxs/Restraint.h>
#include <IMP/log.h>

#include <IMP/atom/Hierarchy.h>
#include <IMP/core/LeavesRefiner.h>

IMPSAXS_BEGIN_NAMESPACE

Restraint::Restraint(const Particles& particles,
                     const Profile* exp_profile, FormFactorType ff_type)
    : IMP::Restraint(IMP::internal::get_model(particles), "SAXS restraint"),
      ff_type_(ff_type) {

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

  IMP_LOG_TERSE("SAXS Restraint::evaluate score\n");

  IMP_NEW(Profile, model_profile, ());
  handler_->compute_profile(model_profile);
  double score = profile_fitter_->compute_score(model_profile);
  bool calc_deriv = acc ? true : false;
  if (!calc_deriv) return score;

  IMP_LOG_TERSE("SAXS Restraint::compute derivatives\n");

  // do we need to resample the curve since it's just been created??
  // yes, since it does not correspond to the experimental one
  IMP_NEW(Profile, resampled_profile, ());
  profile_fitter_->resample(model_profile, resampled_profile);

  Vector<double> effect_size;  // Gaussian model-specific derivative weights
  double offset = 0.0;
  double c = profile_fitter_->compute_scale_factor(model_profile);
  derivative_calculator_->compute_gaussian_effect_size(model_profile, c, offset,
                                                       effect_size);

  handler_->compute_derivatives(derivative_calculator_, model_profile,
                                effect_size, acc);

  IMP_LOG_TERSE("SAXS Restraint::done derivatives, score " << score
                                                                   << "\n");
  return score;
}

namespace {
  const char *get_ff_type_string(FormFactorType ff_type) {
    switch(ff_type) {
      case ALL_ATOMS:
        return "all atoms";
      case HEAVY_ATOMS:
        return "heavy atoms";
      case CA_ATOMS:
        return "Calpha atoms";
      case RESIDUES:
        return "residues";
      default:
        return "unknown";
    }
  }
}

RestraintInfo *Restraint::get_static_info() const {
  IMP_NEW(RestraintInfo, ri, ());
  const Profile *p = profile_fitter_->get_profile();
  ri->add_string("type", "IMP.saxs.Restraint");
  ri->add_string("form factor type", get_ff_type_string(ff_type_));
  ri->add_filename("filename", p->get_name());
  ri->add_float("min q", p->get_min_q());
  ri->add_float("max q", p->get_max_q());
  ri->add_float("delta q", p->get_delta_q());
  return ri.release();
}

IMPSAXS_END_NAMESPACE
