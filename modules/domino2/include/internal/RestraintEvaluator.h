/**
 *  \file RestraintEvaluator.h   \brief A restraint evaluataor interface class
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO2_RESTRAINT_EVALUATOR_H
#define IMPDOMINO2_RESTRAINT_EVALUATOR_H


#include "../domino2_config.h"
#include "DiscreteSampler.h"
#include "CombState.h"
#include "../subset_evaluators.h"
#include <IMP/base_types.h>
#include <vector>
#include <IMP/ScoreState.h>
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/internal/OwnerPointer.h>

IMPDOMINO2_BEGIN_INTERNAL_NAMESPACE

//! Restraint Evaluator interface class
/**
 */
class IMPDOMINO2EXPORT RestraintEvaluator
{
  IMP::internal::OwnerPointer<ModelSubsetEvaluatorTable> mevt_;
  Pointer<ParticleStatesTable> pst_;
public:
  RestraintEvaluator(Model *m, ParticleStatesTable *pst);
  //! Score a restraint with a specific combination of states
  /** \param [in] comb_states the combination of states to calculate
                              scores for
      \param [in] ps the particles that scored by the restraint
      \param [out] comb_values the value of the restraint when the
                               particles are in a combination defined by cs.
   */
  virtual void calc_scores(const Combinations &comb_states,
                           CombinationValues &comb_values,
                           container::ListSingletonContainer *ps);
  virtual void show(std::ostream& out = std::cout) const {}
};
/**
   Problem, restraints that only involve some particles from the subset
   will contaminate values.
 */

IMPDOMINO2_END_INTERNAL_NAMESPACE

#endif /* IMPDOMINO2_RESTRAINT_EVALUATOR_H */
