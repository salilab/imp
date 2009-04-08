/**
 *  \file PairScore.h    \brief A Score on a pair of particles.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMP_PAIR_SCORE_H
#define IMP_PAIR_SCORE_H

#include "config.h"
#include "base_types.h"
#include "RefCounted.h"
#include "Particle.h"
#include "DerivativeAccumulator.h"

IMP_BEGIN_NAMESPACE

//! Abstract score function for a pair of particles.
/** PairScores should take a UnaryFunction as their first
    argument if such is needed.
*/
class IMPEXPORT PairScore : public RefCounted, public Object
{
public:
  PairScore();
  //! Compute the score for the pair and the derivative if needed.
  virtual Float evaluate(Particle *a, Particle *b,
                         DerivativeAccumulator *da) const = 0;
  //! Print information for the PairScore.
  /** Should end in a newline. */
  virtual void show(std::ostream &out=std::cout) const = 0;
  IMP_REF_COUNTED_DESTRUCTOR(PairScore)
};

IMP_OUTPUT_OPERATOR(PairScore);

IMP_END_NAMESPACE

#endif  /* IMP_PAIR_SCORE_H */
