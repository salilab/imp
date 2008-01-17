/**
 *  \file PairScore.h    \brief A Score on a pair of particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_PAIR_SCORE_H
#define __IMP_PAIR_SCORE_H

#include "IMP_config.h"
#include "base_types.h"
#include "Object.h"
#include "Particle.h"
#include "DerivativeAccumulator.h"

namespace IMP
{

//! Abstract score function for a pair of particles.
class IMPDLLEXPORT PairScore : public Object
{
public:
  PairScore() {}
  virtual ~PairScore() {}
  //! Compute the score for the pair and the derivative if needed.
  virtual Float evaluate(Particle *a, Particle *b,
                         DerivativeAccumulator *da) = 0;
  virtual void show(std::ostream &out=std::cout) const = 0;
};

} // namespace IMP

#endif  /* __IMP_PAIR_SCORE_H */
