/**
 *  \file IMP/misc/LowestRefinedPairScore.h
 *  \brief Score on the lowest scoring pair of the refined pairs.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPMISC_LOWEST_REFINED_PAIR_SCORE_H
#define IMPMISC_LOWEST_REFINED_PAIR_SCORE_H

#include <IMP/misc/misc_config.h>
#include <IMP/PairScore.h>
#include <IMP/UnaryFunction.h>
#include <IMP/base/Pointer.h>
#include <IMP/Refiner.h>
#include <IMP/pair_macros.h>

IMPMISC_BEGIN_NAMESPACE

//! Refine both particles with the refiner and score on the lowest pair.
/** Score on the lowest of the pairs defined by refining the two particles.
 */
class IMPMISCEXPORT LowestRefinedPairScore : public PairScore
{
  IMP::base::OwnerPointer<Refiner> r_;
  IMP::base::OwnerPointer<PairScore> f_;
public:
  /** \param[in] r The Refiner to call on each particle
      \param[in] f The pair score to apply to the generated pairs
   */
  LowestRefinedPairScore(Refiner *r, PairScore *f);
  double evaluate_index(Model *m, const ParticleIndexPair& p,
                         DerivativeAccumulator *da) const IMP_OVERRIDE;
  ModelObjectsTemp do_get_inputs(Model *m,
                                 const ParticleIndexes &pis) const;
  IMP_PAIR_SCORE_METHODS(LowestRefinedPairScore);
  IMP_OBJECT_METHODS(LowestRefinedPairScore);;
};

IMPMISC_END_NAMESPACE

#endif  /* IMPMISC_LOWEST_REFINED_PAIR_SCORE_H */
