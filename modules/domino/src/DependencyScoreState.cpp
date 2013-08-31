/**
 *  \file ConjugateGradients.cpp  \brief Simple conjugate gradients optimizer.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino/DependencyScoreState.h>
#include <IMP/kernel/Particle.h>
IMPDOMINO_BEGIN_NAMESPACE
DependencyScoreState::DependencyScoreState() {}

void DependencyScoreState::do_before_evaluate() {}
void DependencyScoreState::do_after_evaluate(DerivativeAccumulator *) {}

ModelObjectsTemp DependencyScoreState::do_get_inputs() const {
  ModelObjectsTemp ret;
  ret += inputp_;
  for (unsigned int i = 0; i < inputc_.size(); ++i) {
    ret +=
        IMP::get_particles(get_model(), inputc_[i]->get_all_possible_indexes());
  }
  ret += inputc_;
  return ret;
}

ModelObjectsTemp DependencyScoreState::do_get_outputs() const {
  ModelObjectsTemp ret;
  ret += outputp_;
  for (unsigned int i = 0; i < outputc_.size(); ++i) {
    ret += IMP::get_particles(get_model(),
                              outputc_[i]->get_all_possible_indexes());
  }
  ret += outputc_;
  return ret;
}

IMPDOMINO_END_NAMESPACE
