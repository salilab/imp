/**
 *  \file LowestRefinedPairScore.h
 *  \brief Score on the lowest scoring pair of the refined pairs.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPMISC_LOWEST_REFINED_PAIR_SCORE_H
#define IMPMISC_LOWEST_REFINED_PAIR_SCORE_H

#include "misc_config.h"
#include <IMP/PairScore.h>
#include <IMP/UnaryFunction.h>
#include <IMP/Pointer.h>
#include <IMP/Refiner.h>

IMPMISC_BEGIN_NAMESPACE

//! Refine both particles with the refiner and score on the lowest pair.
/** Score on the lowest of the pairs defined by refining the two particles.
 */
class IMPMISCEXPORT LowestRefinedPairScore : public PairScore
{
  IMP::internal::OwnerPointer<Refiner> r_;
  IMP::internal::OwnerPointer<PairScore> f_;
public:
  /** \param[in] r The Refiner to call on each particle
      \param[in] f The pair score to apply to the generated pairs
   */
  LowestRefinedPairScore(Refiner *r, PairScore *f);
  ParticlePair get_lowest_refined_pair(const ParticlePair &in) const;
  IMP_PAIR_SCORE(LowestRefinedPairScore);
};

IMPMISC_END_NAMESPACE

#endif  /* IMPMISC_LOWEST_REFINED_PAIR_SCORE_H */
