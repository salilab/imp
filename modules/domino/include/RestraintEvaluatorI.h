/**
 *  \file RestraintEvaluatorI.h   \brief A restraint evaluataor interface class
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO_RESTRAINT_EVALUATOR_I_H
#define IMPDOMINO_RESTRAINT_EVALUATOR_I_H


#include "domino_config.h"
#include "DiscreteSampler.h"
#include "CombState.h"
#include <IMP/base_types.h>
#include <vector>
#include <IMP/ScoreState.h>
#include <IMP/container/ListSingletonContainer.h>

IMPDOMINO_BEGIN_NAMESPACE

//! Restraint Evaluator interface class
/**
 */
class IMPDOMINOEXPORT RestraintEvaluatorI
{
public:
  virtual ~RestraintEvaluatorI(){}
  //! Score a restraint with a specific combination of states
  /** \param [in] comb_states the combination of states to calculate
                              scores for
      \param [in] r the restraint to score
      \param [in] ps the particles that scored by the restraint
      \param [out] comb_values the value of the restraint when the
                               particles are in a combination defined by cs.
   */
  virtual void calc_scores(const Combinations &comb_states,
                           CombinationValues &comb_values,
                           Restraint *r,
                           container::ListSingletonContainer *ps)=0;
  virtual void show(std::ostream& out = std::cout) const=0;
};


IMPDOMINO_END_NAMESPACE

#endif /* IMPDOMINO_RESTRAINT_EVALUATOR_I_H */
