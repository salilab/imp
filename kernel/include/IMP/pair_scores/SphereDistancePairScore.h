/**
 *  \file SphereDistancePairScore.h    
 *  \brief A score on the distance between the surfaces of two spheres.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_SPHERE_DISTANCE_PAIR_SCORE_H
#define __IMP_SPHERE_DISTANCE_PAIR_SCORE_H

#include "../PairScore.h"
#include "../internal/ObjectPointer.h"
#include "../UnaryFunction.h"

namespace IMP
{

//! A score on the distance between the surfaces of two spheres.
/** \ingroup pairscore
 */
class IMPDLLEXPORT SphereDistancePairScore : public PairScore
{
  internal::ObjectPointer<UnaryFunction, true> f_;
  FloatKey radius_;
public:
  SphereDistancePairScore(UnaryFunction *f, 
                          FloatKey radius=FloatKey("radius"));
  virtual ~SphereDistancePairScore(){}
  virtual Float evaluate(Particle *a, Particle *b,
                         DerivativeAccumulator *da);
  virtual void show(std::ostream &out=std::cout) const;
};

} // namespace IMP

#endif  /* __IMP_SPHERE_DISTANCE_PAIR_SCORE_H */
