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
                                       PairScore *ps,
                                       Float md) : nbl_(nbl), sf_(ps),
                                                   max_dist_(md)
{
}


Float NonbondedRestraint::evaluate(DerivativeAccumulator *accum)
{

  IMP_CHECK_OBJECT(sf_.get());
  IMP_CHECK_OBJECT(nbl_);
  Float score=0;
  IMP_LOG(VERBOSE, "Nonbonded restraint on "
          << std::distance(nbl_->nonbonded_begin(max_dist_),
                           nbl_->nonbonded_end(max_dist_))
          << " pairs" << std::endl);
  for (NonbondedListScoreState::NonbondedIterator it 
         = nbl_->nonbonded_begin(max_dist_);
       it != nbl_->nonbonded_end(max_dist_); ++it) {
    float thisscore = sf_->evaluate(it->first, it->second, accum);
    if (thisscore != 0) {
      IMP_LOG(VERBOSE, "Pair " << it->first->get_index() 
              << " and " << it->second->get_index() << " have score "
              << thisscore << std::endl);
    }
    score+= thisscore;
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
