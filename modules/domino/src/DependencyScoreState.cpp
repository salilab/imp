/**
 *  \file ConjugateGradients.cpp  \brief Simple conjugate gradients optimizer.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino/DependencyScoreState.h>

IMPDOMINO_BEGIN_NAMESPACE
DependencyScoreState::DependencyScoreState(){}


void DependencyScoreState::do_before_evaluate() {}
void DependencyScoreState::do_after_evaluate(DerivativeAccumulator *) {}
ContainersTemp DependencyScoreState::get_input_containers() const {
  return inputc_;
}
ContainersTemp DependencyScoreState::get_output_containers() const {
  return outputc_;
}
ParticlesTemp DependencyScoreState::get_input_particles() const {
  ParticlesTemp ret=inputp_;
  for (unsigned int i=0; i< inputc_.size(); ++i) {
    ParticlesTemp cp= inputc_[i]->get_contained_particles();
    ret.insert(ret.end(), cp.begin(), cp.end());
  }
  return ret;
}
ParticlesTemp DependencyScoreState::get_output_particles() const {
  ParticlesTemp ret=outputp_;
  for (unsigned int i=0; i< outputc_.size(); ++i) {
    ParticlesTemp cp= outputc_[i]->get_contained_particles();
    ret.insert(ret.end(), cp.begin(), cp.end());
  }
  return ret;
}
void DependencyScoreState::do_show(std::ostream &) const {}


IMPDOMINO_END_NAMESPACE
