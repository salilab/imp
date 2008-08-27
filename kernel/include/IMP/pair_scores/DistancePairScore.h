/**
 *  \file DistancePairScore.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_DISTANCE_PAIR_SCORE_H
#define __IMP_DISTANCE_PAIR_SCORE_H

#include "../PairScore.h"
#include "../UnaryFunction.h"
#include "../Pointer.h"

namespace IMP
{

//! Apply a function to the distance between two particles.
/** \ingroup pairscore
 */
class IMPDLLEXPORT DistancePairScore : public PairScore
{
  Pointer<UnaryFunction> f_;
public:
  DistancePairScore(UnaryFunction *f);
  virtual ~DistancePairScore(){}
  virtual Float evaluate(Particle *a, Particle *b,
                         DerivativeAccumulator *da) const;
  virtual void show(std::ostream &out=std::cout) const;
};

} // namespace IMP

#endif  /* __IMP_DISTANCE_PAIR_SCORE_H */
