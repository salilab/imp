/**
 *  \file ClosePairsPairScore.h
 *  \brief Apply a PairScore to close pairs.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_CLOSE_PAIRS_PAIR_SCORE_H
#define IMPCORE_CLOSE_PAIRS_PAIR_SCORE_H

#include "core_config.h"
#include "XYZR.h"
#include "RigidClosePairsFinder.h"
#include "internal/CoreListSingletonContainer.h"

#include <IMP/PairScore.h>
#include <IMP/UnaryFunction.h>
#include <IMP/Pointer.h>
#include <IMP/Refiner.h>

IMPCORE_BEGIN_NAMESPACE
/** Apply the score to either the k closest pairs (sphere distance).
    \see ClosePairsScoreState
 */
class IMPCOREEXPORT KClosePairsPairScore : public PairScore
{
  IMP::internal::OwnerPointer<Refiner> r_;
  IMP::internal::OwnerPointer<PairScore> f_;
  int k_;
  mutable double last_distance_;
  IMP::internal::OwnerPointer<RigidClosePairsFinder> cpf_;
public:
  /** only score the k closest pairs.
   */
  KClosePairsPairScore(PairScore *f, Refiner *r,
                      int k=1);

  ParticlePairsTemp get_close_pairs(const ParticlePair &pp) const;

  IMP_PAIR_SCORE(KClosePairsPairScore);
};



/** Apply the score to all pairs whose
    spheres are within a certain distance threshold.

    \see ClosePairsScoreState
 */
class IMPCOREEXPORT ClosePairsPairScore : public PairScore
{
  IMP::internal::OwnerPointer<Refiner> r_;
  IMP::internal::OwnerPointer<PairScore> f_;
  Float th_;
  IMP::internal::OwnerPointer<RigidClosePairsFinder> cpf_;
public:
  /** \param[in] r The Refiner to call on each particle
      \param[in] f The pair score to apply to the generated pairs
      \param[in] max_distance Only score pairs which are close than
      the max_distance
   */
  ClosePairsPairScore(PairScore *f, Refiner *r,
                      Float max_distance);

  ParticlePairsTemp get_close_pairs(const ParticlePair &pp) const;

  IMP_PAIR_SCORE(ClosePairsPairScore);
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_CLOSE_PAIRS_PAIR_SCORE_H */
