/**
 *  \file NonbondedRestraint.cpp 
 *  \brief A nonbonded restraint.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <cmath>

#include "IMP/PairScore.h"
#include "IMP/log.h"
#include "IMP/restraints/NonbondedRestraint.h"
#include "IMP/score_states/NonbondedListScoreState.h"

namespace IMP
{

NonbondedRestraint::NonbondedRestraint(NonbondedListScoreState *nbl, 
                                       PairScore *ps) : nbl_(nbl), sf_(ps)
{
}


Float NonbondedRestraint::evaluate(DerivativeAccumulator *accum)
{

  IMP_CHECK_OBJECT(sf_);
  IMP_CHECK_OBJECT(nbl_);
  Float score=0;

  for (NonbondedListScoreState::NonbondedIterator it = nbl_->nonbonded_begin();
       it != nbl_->nonbonded_end(); ++it) {
    score += sf_->evaluate(it->first, it->second, accum);
  }

  return score;
}


void NonbondedRestraint::show(std::ostream& out) const
{
  out << "Nonbonded restraint with score function ";
  sf_->show(out);
  out << std::endl;
}

} // namespace IMP
