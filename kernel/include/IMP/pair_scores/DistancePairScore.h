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

namespace IMP
{

//! Apply a function to the distance between two particles.
class IMPDLLEXPORT DistancePairScore : public PairScore
{
  std::auto_ptr<UnaryFunction> f_;
public:
  DistancePairScore(UnaryFunction *f);
  virtual ~DistancePairScore(){}
  virtual Float evaluate(Particle *a, Particle *b,
                         DerivativeAccumulator *da);
  virtual void show(std::ostream &out=std::cout) const;
};

namespace internal
{

//! An internal helper function for evaluating distance potentials
/** The function applies f to scale*(distance-offset).
 */
Float evaluate_distance_pair_score(Particle *a, Particle *b,
                                   DerivativeAccumulator *da,
                                   UnaryFunction *f,
                                   Float offset,
                                   Float scale);

} // namespace internal

} // namespace IMP

#endif  /* __IMP_DISTANCE_PAIR_SCORE_H */
