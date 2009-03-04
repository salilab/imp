/**
 *  \file RefineOncePairScore.h
 *  \brief Refine particles at most once with a ParticleRefiner.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPMISC_REFINE_ONCE_PAIR_SCORE_H
#define IMPMISC_REFINE_ONCE_PAIR_SCORE_H

#include "config.h"

#include <IMP/PairScore.h>
#include <IMP/UnaryFunction.h>
#include <IMP/Pointer.h>
#include <IMP/ParticleRefiner.h>

IMPMISC_BEGIN_NAMESPACE

#ifndef IMP_NO_DEPRECATED

//! Refine the input particles at most once with the ParticleRefiner.
/** Each passed particle is refined once before the resulting pairs
    have the pair score called on them.
    \deprecated Use core::RefinedPairsPairScore
 */
class IMPMISCEXPORT RefineOncePairScore : public PairScore
{
  Pointer<ParticleRefiner> r_;
  Pointer<PairScore> f_;

public:
  /** \param[in] r The ParticleRefiner to call on each particle
      \param[in] f The pair score to apply to the generated pairs
   */
  RefineOncePairScore(ParticleRefiner *r, PairScore *f);
  /** */
  virtual ~RefineOncePairScore(){}
  /** */
  virtual Float evaluate(Particle *a, Particle *b,
                         DerivativeAccumulator *da) const;
  /** */
  virtual void show(std::ostream &out=std::cout) const;
};

#endif // IMP_NO_DEPRECATED

IMPMISC_END_NAMESPACE

#endif  /* IMPMISC_REFINE_ONCE_PAIR_SCORE_H */
