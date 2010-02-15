/**
 *  \file DistanceToSingletonScore.h
 *  \brief A Score on the distance to a fixed point.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_DISTANCE_TO_SINGLETON_SCORE_H
#define IMPCORE_DISTANCE_TO_SINGLETON_SCORE_H

#include "config.h"
#include <IMP/algebra/Vector3D.h>
#include <IMP/SingletonScore.h>
#include <IMP/Pointer.h>
#include <IMP/UnaryFunction.h>

IMPCORE_BEGIN_NAMESPACE

//! Apply a function to the distance to a fixed point.
/** A particle is scored based on the distance between it and a constant
    point as passed to a UnaryFunction. This is useful for anchoring
    constraining particles within a sphere.

    To restrain a set of particles store in SingletonContainer pc in a sphere
    do the following:
    \verbinclude fragments/restrain_in_sphere.py
 */
class IMPCOREEXPORT DistanceToSingletonScore : public SingletonScore
{
  IMP::internal::OwnerPointer<UnaryFunction> f_;
  algebra::Vector3D pt_;
public:
  DistanceToSingletonScore(UnaryFunction *f, const algebra::Vector3D& pt);
  IMP_SIMPLE_SINGLETON_SCORE(DistanceToSingletonScore);
};



//! Apply a function to the distance to a fixed point.
/** A particle is scored based on the distance between it and a constant
    point as passed to a UnaryFunction. This is useful for anchoring
    constraining particles within a sphere.

    To restrain a set of particles store in SingletonContainer pc in a sphere
    do the following:
    \verbinclude fragments/restrain_in_sphere.py
 */
class IMPCOREEXPORT SphereDistanceToSingletonScore : public SingletonScore
{
  IMP::internal::OwnerPointer<UnaryFunction> f_;
  algebra::Vector3D pt_;
public:
  SphereDistanceToSingletonScore(UnaryFunction *f, const algebra::Vector3D& pt);
  IMP_SIMPLE_SINGLETON_SCORE(SphereDistanceToSingletonScore);
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_DISTANCE_TO_SINGLETON_SCORE_H */
