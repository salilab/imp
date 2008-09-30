/**
 *  \file AngleTripletScore.h
 *  \brief A Score on the angle between three of particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_ANGLE_TRIPLET_SCORE_H
#define __IMP_ANGLE_TRIPLET_SCORE_H

#include "../TripletScore.h"
#include "../UnaryFunction.h"
#include "../Pointer.h"

IMP_BEGIN_NAMESPACE

//! Apply a function to the angle between three particles.
/** \ingroup triplet
 */
class IMPDLLEXPORT AngleTripletScore : public TripletScore
{
  Pointer<UnaryFunction> f_;
public:
  AngleTripletScore(UnaryFunction *f);
  virtual ~AngleTripletScore(){}
  virtual Float evaluate(Particle *a, Particle *b, Particle *c,
                         DerivativeAccumulator *da) const;
  virtual void show(std::ostream &out=std::cout) const;
};

IMP_END_NAMESPACE

#endif  /* __IMP_ANGLE_TRIPLET_SCORE_H */
