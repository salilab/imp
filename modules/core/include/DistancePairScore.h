/**
 *  \file DistancePairScore.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMPCORE_DISTANCE_PAIR_SCORE_H
#define __IMPCORE_DISTANCE_PAIR_SCORE_H

#include "core_exports.h"

#include <IMP/PairScore.h>
#include <IMP/UnaryFunction.h>
#include <IMP/Pointer.h>

IMPCORE_BEGIN_NAMESPACE

//! Apply a function to the distance between two particles.
/** \ingroup pairscore
 */
class IMPCOREEXPORT DistancePairScore : public PairScore
{
  Pointer<UnaryFunction> f_;
public:
  DistancePairScore(UnaryFunction *f);
  virtual ~DistancePairScore(){}
  virtual Float evaluate(Particle *a, Particle *b,
                         DerivativeAccumulator *da) const;
  virtual void show(std::ostream &out=std::cout) const;
};

IMPCORE_END_NAMESPACE

#endif  /* __IMPCORE_DISTANCE_PAIR_SCORE_H */
