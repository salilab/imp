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
/** The process is paraemterized by
    - a ParticleStatesTable which
    stores the set of states for each particle
    - a SubsetStatesTable which produces a list of states for
    a given subset of the particles
    - and a SubsetEvaluatorTable which scores a state of a
    a given subset.
    .
    Defaults of DefaultSubsetStatesTable and CachingModelSubsetEvalauatorTable
    are provided for the last two.

    \note the restraint scores must be non-negative for domino.
    If you are using restraints which can produce negative values,
    we can provide a restraint which wraps another and makes
    it non-negative. Ping us.
 */
class IMPDOMINO2EXPORT DominoSampler : public Sampler
{
  IMP::internal::OwnerPointer<ParticleStatesTable> enumerators_;
  IMP::internal::OwnerPointer<SubsetStatesTable> node_enumerators_;
  IMP::internal::OwnerPointer<SubsetEvaluatorTable> evaluators_;
public:
  DominoSampler(Model *m);
  DominoSampler(Model*m, ParticleStatesTable *pst);
  /** Particle states can be set either using this method,
      or equivalently, by accessing the table itself
      using get_particle_states_table(). This method
      is provided for users who want to use the default values
      and want a simple inferface.*/
  void set_particle_states(Particle *p, ParticleStates *se);

  /** \name Advanced
      Default values are provided, you only need to replace these
      if you want to do something special. See the overview of
      the module for a general description.
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
