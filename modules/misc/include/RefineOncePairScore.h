/**
 *  \file RefineOncePairScore.h
 *  \brief Refine particles at most once with a Refiner.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#ifndef IMPMISC_REFINE_ONCE_PAIR_SCORE_H
#define IMPMISC_REFINE_ONCE_PAIR_SCORE_H

#include "config.h"

#include <IMP/PairScore.h>
#include <IMP/UnaryFunction.h>
#include <IMP/Pointer.h>
#include <IMP/Refiner.h>

IMPMISC_BEGIN_NAMESPACE

#ifndef IMP_NO_DEPRECATED

//! Refine the input particles at most once with the Refiner.
/** Each passed particle is refined once before the resulting pairs
    have the pair score called on them.
    \deprecated Use core::RefinedPairsPairScore
 */
class IMPMISCEXPORT RefineOncePairScore : public PairScore
{
  IMP::internal::OwnerPointer<Refiner> r_;
  IMP::internal::OwnerPointer<PairScore> f_;
public:
  /** \param[in] r The Refiner to call on each particle
      \param[in] f The pair score to apply to the generated pairs
   */
  RefineOncePairScore(Refiner *r, PairScore *f);
  IMP_PAIR_SCORE(RefineOncePairScore, get_module_version_info());
};

#endif // IMP_NO_DEPRECATED

IMPMISC_END_NAMESPACE

#endif  /* IMPMISC_REFINE_ONCE_PAIR_SCORE_H */
