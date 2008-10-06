/**
 *  \file DistanceToSingletonScore.h
 *  \brief A Score on the distance to a fixed point.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMPCORE_DISTANCE_TO_SINGLETON_SCORE_H
#define __IMPCORE_DISTANCE_TO_SINGLETON_SCORE_H

#include "core_exports.h"

#include <IMP/SingletonScore.h>
#include <IMP/Vector3D.h>
#include <IMP/Pointer.h>
#include <IMP/UnaryFunction.h>

IMPCORE_BEGIN_NAMESPACE

//! Apply a function to the distance to a fixed point.
/** \ingroup singleton
 */
class IMPCOREEXPORT DistanceToSingletonScore : public SingletonScore
{
  Pointer<UnaryFunction> f_;
  Vector3D pt_;
public:
  DistanceToSingletonScore(UnaryFunction *f, const Vector3D& pt);
  virtual ~DistanceToSingletonScore(){}
  virtual Float evaluate(Particle *a,
                         DerivativeAccumulator *da) const;
  virtual void show(std::ostream &out=std::cout) const;
};

IMPCORE_END_NAMESPACE

#endif  /* __IMPCORE_DISTANCE_TO_SINGLETON_SCORE_H */
