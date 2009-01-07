/**
 *  \file NonbondedRestraint.cpp
 *  \brief A nonbonded restraint.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/NonbondedRestraint.h>
#include <IMP/core/NonbondedListScoreState.h>
#include <IMP/deprecation.h>

#include <IMP/PairScore.h>
#include <IMP/log.h>

#include <cmath>

IMPCORE_BEGIN_NAMESPACE

NonbondedRestraint::NonbondedRestraint(PairScore *ps,
                                       NonbondedListScoreState *nbl)
  : nbl_(nbl), sf_(ps)
{
  IMP_DEPRECATED(NonbondedRestraint, ParticlePairsRestraint);
}


Float NonbondedRestraint::evaluate(DerivativeAccumulator *accum)
{

  IMP_CHECK_OBJECT(sf_.get());
  IMP_CHECK_OBJECT(nbl_);
  Float score=0;
  IMP_LOG(VERBOSE, "Nonbonded restraint on "
          << std::distance(nbl_->nonbonded_begin(),
                           nbl_->nonbonded_end())
          << " pairs" << std::endl);
  for (NonbondedListScoreState::NonbondedIterator it
         = nbl_->nonbonded_begin();
       it != nbl_->nonbonded_end(); ++it) {
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

ParticlesList NonbondedRestraint::get_interacting_particles() const
{
  IMP_failure("Not implemented yet because not needed", ErrorException);
  return ParticlesList();
}

void NonbondedRestraint::show(std::ostream& out) const
{
  out << "Nonbonded restraint with score function ";
  sf_->show(out);
  out << std::endl;
}

IMPCORE_END_NAMESPACE
