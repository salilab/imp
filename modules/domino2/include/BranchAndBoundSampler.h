/**
 *  \file domino2/BranchAndBoundSampler.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO2_BRANCH_AND_BOUND_SAMPLER_H
#define IMPDOMINO2_BRANCH_AND_BOUND_SAMPLER_H

#include "domino2_config.h"
//#include "Evaluator.h"
#include "subset_states.h"
#include "particle_states.h"
#include "subset_evaluators.h"
#include <IMP/Sampler.h>
#include <IMP/macros.h>
#include <IMP/internal/OwnerPointer.h>

IMPDOMINO2_BEGIN_NAMESPACE



//! Sample best solutions using BranchAndBound
/** Find all good configurations of the model using branch and bound.
    Searches are truncated when the score is worse than the the thresholds
    in the Sampler or when two particles with the same ParticlesState
    are assigned the same state.
 */
class IMPDOMINO2EXPORT BranchAndBoundSampler : public Sampler
{
  IMP::internal::OwnerPointer<ParticleStatesTable> enumerators_;
  IMP::internal::OwnerPointer<SubsetEvaluatorTable> evaluators_;
public:
  BranchAndBoundSampler(Model *m);
  BranchAndBoundSampler(Model*m, ParticleStatesTable *pst);
  void set_particle_states(Particle *p, ParticleStates *se);
  /** \name Advanced
      Default values are provided, you only need to replace these
      if you want to do something special. See the overview of
      the module for a general description.
      @{
   */
  void set_subset_evaluator_table(SubsetEvaluatorTable *eval);
  void set_particle_states_table(ParticleStatesTable *cse) {
    enumerators_= cse;
  }
  SubsetEvaluatorTable* get_subset_evaluator_table() const {
    return evaluators_;
  }
  ParticleStatesTable* get_particle_states_table() const {
    return enumerators_;
  }
  /** @} */
  IMP_SAMPLER(BranchAndBoundSampler);
};


IMP_OBJECTS(BranchAndBoundSampler, BranchAndBoundSamplers);


IMPDOMINO2_END_NAMESPACE

#endif  /* IMPDOMINO2_BRANCH_AND_BOUND_SAMPLER_H */
