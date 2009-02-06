/**
 *  \file RefinePairsPairScore.h
 *  \brief Generate pairs by applying a ParticleRefiner to the paricles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_REFINED_PAIRS_PAIR_SCORE_H
#define IMPCORE_REFINED_PAIRS_PAIR_SCORE_H

#include "config.h"

#include <IMP/PairScore.h>
#include <IMP/UnaryFunction.h>
#include <IMP/Pointer.h>
#include <IMP/ParticleRefiner.h>

IMPCORE_BEGIN_NAMESPACE

//! Generate pairs to score by applying a ParticleRefiner.
/** Each passed particle refined and then the PairScore applied
    to all resulting pairs.
 */
class IMPCOREEXPORT RefinedPairsPairScore : public PairScore
{
  Pointer<ParticleRefiner> r_;
  Pointer<PairScore> f_;
public:
  /** \param[in] r The ParticleRefiner to call on each particle
      \param[in] f The pair score to apply to the generated pairs
   */
  RefinedPairsPairScore(ParticleRefiner *r, PairScore *f);
  virtual ~RefinedPairsPairScore(){}
  virtual Float evaluate(Particle *a, Particle *b,
                         DerivativeAccumulator *da) const;
  virtual void show(std::ostream &out=std::cout) const;
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_REFINED_PAIRS_PAIR_SCORE_H */
