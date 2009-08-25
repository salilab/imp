/**
 *  \file RefinedPairsPairScore.h
 *  \brief Generate pairs by applying a Refiner to the paricles.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_REFINED_PAIRS_PAIR_SCORE_H
#define IMPCORE_REFINED_PAIRS_PAIR_SCORE_H

#include "config.h"
#include <IMP/PairScore.h>
#include <IMP/UnaryFunction.h>
#include <IMP/Pointer.h>
#include <IMP/Refiner.h>

IMPCORE_BEGIN_NAMESPACE

//! Generate pairs to score by applying a Refiner.
/** Each passed particle refined and then the PairScore applied
    to all resulting pairs.
 */
class IMPCOREEXPORT RefinedPairsPairScore : public PairScore
{
  Pointer<Refiner> r_;
  Pointer<PairScore> f_;
public:
  /** \param[in] r The Refiner to call on each particle
      \param[in] f The pair score to apply to the generated pairs
   */
  RefinedPairsPairScore(Refiner *r, PairScore *f);
  IMP_PAIR_SCORE(RefinedPairsPairScore, get_module_version_info());
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_REFINED_PAIRS_PAIR_SCORE_H */
