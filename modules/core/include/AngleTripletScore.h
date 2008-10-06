/**
 *  \file AngleTripletScore.h
 *  \brief A Score on the angle between three of particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMPCORE_ANGLE_TRIPLET_SCORE_H
#define __IMPCORE_ANGLE_TRIPLET_SCORE_H

#include "core_exports.h"

#include <IMP/TripletScore.h>
#include <IMP/UnaryFunction.h>
#include <IMP/Pointer.h>

IMPCORE_BEGIN_NAMESPACE

//! Apply a function to the angle between three particles.
/** \ingroup triplet
 */
class IMPCOREEXPORT AngleTripletScore : public TripletScore
{
  Pointer<UnaryFunction> f_;
public:
  AngleTripletScore(UnaryFunction *f);
  virtual ~AngleTripletScore(){}
  virtual Float evaluate(Particle *a, Particle *b, Particle *c,
                         DerivativeAccumulator *da) const;
  virtual void show(std::ostream &out=std::cout) const;
};

IMPCORE_END_NAMESPACE

#endif  /* __IMPCORE_ANGLE_TRIPLET_SCORE_H */
