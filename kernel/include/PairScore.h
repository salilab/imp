/**
 *  \file PairScore.h    \brief A Score on a pair of particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_PAIR_SCORE_H
#define __IMP_PAIR_SCORE_H

#include "IMP_config.h"
#include "base_types.h"
#include "RefCountedObject.h"
#include "Particle.h"
#include "DerivativeAccumulator.h"

IMP_BEGIN_NAMESPACE

/** \ingroup restraint
    \addtogroup pairscore Score functions on two particles
    Score functions to by applied to a pair of particles. These can be
    used to make more flexible restraints.
 */

//! Abstract score function for a pair of particles.
/** PairScores should take a UnaryFunction as their first
    argument if such is needed.
*/
class IMPDLLEXPORT PairScore : public RefCountedObject
{
public:
  PairScore() {}
  virtual ~PairScore();
  //! Compute the score for the pair and the derivative if needed.
  virtual Float evaluate(Particle *a, Particle *b,
                         DerivativeAccumulator *da) const = 0;
  virtual void show(std::ostream &out=std::cout) const = 0;
};

IMP_OUTPUT_OPERATOR(PairScore);

IMP_END_NAMESPACE

#endif  /* __IMP_PAIR_SCORE_H */
