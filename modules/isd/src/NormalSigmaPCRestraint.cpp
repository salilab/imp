/**
 *  \file isd/NormalSigmaPCRestraint.cpp
 *  \brief A Penalized Complexity prior on sigma of a normal distribution.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/NormalSigmaPCRestraint.h>
#include <cmath>

IMPISD_BEGIN_NAMESPACE

NormalSigmaPCRestraint::NormalSigmaPCRestraint(Model *m,
                                               ParticleIndex spi,
                                               double su,
                                               double alpha,
                                               std::string name)
    : Restraint(m, name), spi_(spi), su_(su), nloga_(-std::log(alpha)) {
  IMP_USAGE_CHECK(Scale::get_is_setup(m, spi), "Sigma must be a Scale.");
  IMP_USAGE_CHECK(su > 0, "Sigma upper must be positive.");
  create_distribution(alpha);
}

NormalSigmaPCRestraint::NormalSigmaPCRestraint(Scale s,
                                               double su,
                                               double alpha,
                                               std::string name)
    : Restraint(s.get_model(), name), spi_(s.get_particle_index())
    , su_(su), nloga_(-std::log(alpha)) {
  IMP_USAGE_CHECK(Scale::get_is_setup(s), "Sigma must be a Scale.");
  IMP_USAGE_CHECK(su > 0, "Sigma upper must be positive.");
  create_distribution(alpha);
}

void NormalSigmaPCRestraint::create_distribution(double alpha) {
  pcp_ = new PenalizedComplexityPrior(1, 1, alpha);
}

double NormalSigmaPCRestraint::unprotected_evaluate(DerivativeAccumulator *accum)
    const {
  double s, score;
  Scale sigma = get_sigma();
  s = sigma.get_scale();

  if (s == 0.) {
    score = std::log(su_ / nloga_);
  } else {
    pcp_->set_Dxi(s * s / su_ / su_);
    pcp_->set_Jxi(2 * s / su_ / su_);
    score = pcp_->evaluate();
  }

  if (accum) {
    // Derivative of PC prior on sigma is a constant, so special-case.
    sigma.add_to_scale_derivative(nloga_ / su_, *accum);
  }

  pcp_->set_was_used(true);
  return score;
}

Scale NormalSigmaPCRestraint::get_sigma() const {
  return Scale(get_model(), spi_);
}

ModelObjectsTemp NormalSigmaPCRestraint::do_get_inputs() const {
  ParticlesTemp ret(1, get_model()->get_particle(spi_));
  return ret;
}

IMPISD_END_NAMESPACE
