/**
 *  \file ConjugateGradients.cpp  \brief Simple conjugate gradients optimizer.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino2/BranchAndBoundSampler.h>
#include <IMP/domino2/subset_states.h>
#include <IMP/domino2/utility.h>

#include <IMP/domino2/internal/inference.h>
#include <IMP/internal/graph_utility.h>
#include <IMP/file.h>

IMPDOMINO2_BEGIN_NAMESPACE


BranchAndBoundSampler::BranchAndBoundSampler(Model *m,
                                             ParticleStatesTable* pst):
  Sampler(m, "BranchAndBound Sampler %1"),
  enumerators_(pst){
}

BranchAndBoundSampler::BranchAndBoundSampler(Model *m):
  Sampler(m, "BranchAndBound Sampler %1"),
  enumerators_(new ParticleStatesTable()){
}



ConfigurationSet *BranchAndBoundSampler::do_sample() const {
  IMP_NEW(DefaultSubsetStatesTable, dsst, (enumerators_));
  IMP::internal::OwnerPointer<SubsetEvaluatorTable> set;
  if (evaluators_) {
    set= evaluators_;
  } else {
    set= new ModelSubsetEvaluatorTable(get_model(),
                                       get_particle_states_table());
    set->set_sampler(this);
  }
  dsst->set_subset_evaluator_table(set);
  const ParticlesTemp pt= enumerators_->get_particles();
  const Subset s(pt);
  IMP::internal::OwnerPointer<const SubsetStates> ss
    = dsst->get_subset_states(s);
  Pointer<ConfigurationSet> ret= new ConfigurationSet(get_model());
  for (unsigned int i=0; i< ss->get_number_of_states(); ++i) {
    SubsetState scur= ss->get_state(i);
    ret->load_configuration(-1);
    for (unsigned int j=0; j< pt.size(); ++j) {
      Particle *p=pt[j];
      Pointer<ParticleStates> ps=enumerators_->get_particle_states(p);
      ps->load_state(scur[j], p);
    }
    if (get_is_good_configuration()) {
      ret->save_configuration();
    }
  }
  return ret.release();
}

void BranchAndBoundSampler::set_particle_states(Particle *p,
                                                ParticleStates *se) {
  enumerators_->set_particle_states(p, se);
}
void BranchAndBoundSampler
::set_subset_evaluator_table(SubsetEvaluatorTable *eval) {
  evaluators_= eval;
  evaluators_->set_sampler(this);
}
void BranchAndBoundSampler::do_show(std::ostream &out) const {
}

IMPDOMINO2_END_NAMESPACE
