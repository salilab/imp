/**
 *  \file RefineOncePairScore.h    
 *  \brief Refine particles at most once with a ParticleRefiner.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_REFINE_ONCE_PAIR_SCORE_H
#define __IMP_REFINE_ONCE_PAIR_SCORE_H

#include "../PairScore.h"
#include "../UnaryFunction.h"
#include "../ParticleRefiner.h"
#include "../Pointer.h"

namespace IMP
{

//! Refine the input particles at most once with the ParticleRefiner.
/** Each passed particle is refined once before the resulting pairs
    have the pair score called on them.

    \ingroup pairscore
 */
class IMPDLLEXPORT RefineOncePairScore : public PairScore
{
  Pointer<ParticleRefiner> r_;
  Pointer<PairScore> f_;

public:
  /** \param[in] r The ParticleRefiner to call on each particle
      \param[in] f The pair score to apply to the generated pairs
   */
  RefineOncePairScore(ParticleRefiner *r, PairScore *f);
  virtual ~RefineOncePairScore(){}
  virtual Float evaluate(Particle *a, Particle *b,
                         DerivativeAccumulator *da) const;
  virtual void show(std::ostream &out=std::cout) const;
};

} // namespace IMP

#endif  /* __IMP_REFINE_ONCE_PAIR_SCORE_H */
