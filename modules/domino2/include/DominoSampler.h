/**
 *  \file domino2/DominoSampler.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO2_DOMINO_SAMPLER_H
#define IMPDOMINO2_DOMINO_SAMPLER_H

#include "domino2_config.h"
//#include "Evaluator.h"
#include "subset_states.h"
#include "particle_states.h"
#include "subset_evaluators.h"
#include <IMP/Sampler.h>
#include <IMP/macros.h>
#include <IMP/internal/OwnerPointer.h>

IMPDOMINO2_BEGIN_NAMESPACE



//! Sample best solutions using Domino
/** \note the restraint scores must be non-negative for domino.
    If you are using restraints which can produce negative values,
    we can provide a restraint which wraps another and makes
    it non-negative.
 */
class IMPDOMINO2EXPORT DominoSampler : public Sampler
{
  IMP::internal::OwnerPointer<ParticleStatesTable> enumerators_;
  IMP::internal::OwnerPointer<SubsetStatesTable> node_enumerators_;
  IMP::internal::OwnerPointer<SubsetEvaluatorTable> evaluators_;
public:
  DominoSampler(Model *m);
  DominoSampler(Model*m, ParticleStatesTable *pst);
  // use these functions to set up the state space for the particles
  void set_particle_states(Particle *p, ParticleStates *se);
  /** \name Advanced
      Default values are provided, you only need to replace these
      if you want to do something special.
      @{
   */
  void set_subset_evaluator_table(SubsetEvaluatorTable *eval);
  void set_subset_states_table(SubsetStatesTable *cse);
  void set_particle_states_table(ParticleStatesTable *cse) {
    enumerators_= cse;
  }
  SubsetEvaluatorTable* get_subset_evaluator_table() const {
    return evaluators_;
  }
  SubsetStatesTable* get_subset_states_table() const {
    return node_enumerators_;
  }
  ParticleStatesTable* get_particle_states_table() const {
    return enumerators_;
  }
  /** @} */
  IMP_SAMPLER(DominoSampler);
};


IMP_OBJECTS(DominoSampler, DominoSamplers);


IMPDOMINO2_END_NAMESPACE

#endif  /* IMPDOMINO2_DOMINO_SAMPLER_H */
