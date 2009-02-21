/**
 *  \file SphereDistancePairScore.h
 *  \brief A score on the distance between the surfaces of two spheres.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_SPHERE_DISTANCE_PAIR_SCORE_H
#define IMPCORE_SPHERE_DISTANCE_PAIR_SCORE_H

#include "config.h"

#include <IMP/PairScore.h>
#include <IMP/Pointer.h>
#include <IMP/UnaryFunction.h>

IMPCORE_BEGIN_NAMESPACE

//! A score on the distance between the surfaces of two spheres.
/** \see XYZRDecorator
    \see DistancePairScore
 */
class IMPCOREEXPORT SphereDistancePairScore : public PairScore
{
  Pointer<UnaryFunction> f_;
  FloatKey radius_;
public:
  /** */
  SphereDistancePairScore(UnaryFunction *f,
                          FloatKey radius=FloatKey("radius"));
  virtual ~SphereDistancePairScore(){}
  virtual Float evaluate(Particle *a, Particle *b,
                         DerivativeAccumulator *da) const;
  virtual void show(std::ostream &out=std::cout) const;
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_SPHERE_DISTANCE_PAIR_SCORE_H */
