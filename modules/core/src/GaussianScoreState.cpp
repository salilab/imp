/**
 *  \file GaussianScoreState.h
 *  \brief Update global covariance of Gaussians
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core/GaussianScoreState.h>

IMPCORE_BEGIN_NAMESPACE
GaussianScoreState::GaussianScoreState(
  kernel::Model *m, const kernel::ParticleIndexes& ps):
  ScoreState(m, "GaussianScoreState%1%"),
  ps_(ps) { }

void GaussianScoreState::do_before_evaluate() {
  for (size_t np=0;np<ps_.size();np++){
    core::Gaussian(get_model(),ps_[np]).update_global_covariance();
  }
}

void GaussianScoreState::do_after_evaluate(DerivativeAccumulator *) {}

ModelObjectsTemp GaussianScoreState::do_get_inputs() const {
  ModelObjectsTemp ret;
  for (size_t i=0;i<ps_.size();i++){
    ret.push_back(get_model()->get_particle(ps_[i]));
  }
  return ret;
}

ModelObjectsTemp GaussianScoreState::do_get_outputs() const {
  return kernel::ModelObjectsTemp();
}

IMPCORE_END_NAMESPACE
