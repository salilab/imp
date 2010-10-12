/**
 *  \file ClosePairsPairScore.h
 *  \brief Apply a PairScore to close pairs.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_CLOSE_PAIRS_PAIR_SCORE_H
#define IMPCORE_CLOSE_PAIRS_PAIR_SCORE_H

#include "core_config.h"
#include "XYZR.h"

#include <IMP/PairScore.h>
#include <IMP/UnaryFunction.h>
#include <IMP/Pointer.h>
#include <IMP/Refiner.h>

IMPCORE_BEGIN_NAMESPACE

//! Traverse the Refiner hierarchy to find all pairs which are close
/** Apply the score to either the k closest pairs or all pairs whose
    centers are within a certain distance threshold.

    \note Either CGAL or ANN must be installed for this to be efficient
   (except when used with non-rigid bodies with k=1)
    \see ClosePairsScoreState
 */
class IMPCOREEXPORT ClosePairsPairScore : public PairScore
{
  IMP::internal::OwnerPointer<Refiner> r_;
  IMP::internal::OwnerPointer<PairScore> f_;
  Float th_;
  int k_;
  FloatKey rk_;
public:
  /** \param[in] r The Refiner to call on each particle
      \param[in] f The pair score to apply to the generated pairs
      \param[in] max_distance Only score pairs which are close than
      the max_distance
      \param[in] rk The key to use for the radius.
   */
  ClosePairsPairScore(PairScore *f, Refiner *r,
                      Float max_distance);
  /** only score the k closest pairs.
   */
  ClosePairsPairScore(PairScore *f, Refiner *r,
                      int k=1);

  IMP_PAIR_SCORE(ClosePairsPairScore);
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_CLOSE_PAIRS_PAIR_SCORE_H */
