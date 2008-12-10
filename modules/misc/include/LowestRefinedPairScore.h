/**
 *  \file LowestRefinedPairScore.h
 *  \brief Score on the lowest scoring pair of the refined pairs.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPMISC_LOWEST_REFINED_PAIR_SCORE_H
#define IMPMISC_LOWEST_REFINED_PAIR_SCORE_H

#include "config.h"

#include <IMP/PairScore.h>
#include <IMP/UnaryFunction.h>
#include <IMP/Pointer.h>
#include <IMP/ParticleRefiner.h>

IMPMISC_BEGIN_NAMESPACE

//! Refine both particles with the refiner and score on the lowest pair.
/** Score on the lowest of the pairs defined by refining the two particles.
 */
class IMPMISCEXPORT LowestRefinedPairScore : public PairScore
{
  Pointer<ParticleRefiner> r_;
  Pointer<PairScore> f_;

public:
  /** \param[in] r The ParticleRefiner to call on each particle
      \param[in] f The pair score to apply to the generated pairs
   */
  LowestRefinedPairScore(ParticleRefiner *r, PairScore *f);
  /** */
  virtual ~LowestRefinedPairScore(){}
  /** */
  virtual Float evaluate(Particle *a, Particle *b,
                         DerivativeAccumulator *da) const;
  /** */
  virtual void show(std::ostream &out=std::cout) const;
};

IMPMISC_END_NAMESPACE

#endif  /* IMPMISC_LOWEST_REFINED_PAIR_SCORE_H */
