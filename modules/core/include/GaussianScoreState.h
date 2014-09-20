/**
 *  \file IMP/core/GaussianScoreState.h
 *  \brief Update global covariance of Gaussians
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_GAUSSIAN_SCORE_STATE_H
#define IMPCORE_GAUSSIAN_SCORE_STATE_H

#include <IMP/core/core_config.h>
#include <IMP/core/Gaussian.h>
#include <IMP/ScoreState.h>


IMPCORE_BEGIN_NAMESPACE

//! Update the global covariance of a Gaussian
class IMPCOREEXPORT GaussianScoreState : public ScoreState {
  kernel::ParticleIndexes ps_;

 public:
  GaussianScoreState(kernel::Model *m, const kernel::ParticleIndexes& ps);
  virtual void do_before_evaluate() IMP_OVERRIDE;
  virtual void do_after_evaluate(DerivativeAccumulator* da) IMP_OVERRIDE;
  virtual kernel::ModelObjectsTemp do_get_inputs() const;
  virtual kernel::ModelObjectsTemp do_get_outputs() const;
  IMP_OBJECT_METHODS(GaussianScoreState);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_GAUSSIAN_SCORE_STATE_H */
