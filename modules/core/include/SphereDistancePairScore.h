/**
 *  \file SphereDistancePairScore.h
 *  \brief A score on the distance between the surfaces of two spheres.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_SPHERE_DISTANCE_PAIR_SCORE_H
#define IMPCORE_SPHERE_DISTANCE_PAIR_SCORE_H

#include "config.h"
#include <IMP/PairScore.h>
#include <IMP/Pointer.h>
#include <IMP/UnaryFunction.h>

IMPCORE_BEGIN_NAMESPACE

//! A score on the distance between the surfaces of two spheres.
/** \see XYZR
    \see DistancePairScore
    \see NormalizedSphereDistancePairScore
 */
class IMPCOREEXPORT SphereDistancePairScore : public PairScore
{
  IMP::internal::OwnerPointer<UnaryFunction> f_;
  FloatKey radius_;
public:
  SphereDistancePairScore(UnaryFunction *f,
                          FloatKey radius=FloatKey("radius"));
  IMP_SIMPLE_PAIR_SCORE(SphereDistancePairScore, get_module_version_info());
};


//! A score on the normalized distance between the surfaces of two spheres
/** The distance between the surfaces of the two spheres is divided by the
    smaller radius to normalize it.
    \see SphereDistancePairScore
    \see XYZR
    \see DistancePairScore
 */
class IMPCOREEXPORT NormalizedSphereDistancePairScore : public PairScore
{
  IMP::internal::OwnerPointer<UnaryFunction> f_;
  FloatKey radius_;
public:
  NormalizedSphereDistancePairScore(UnaryFunction *f,
                                    FloatKey radius=FloatKey("radius"));
  IMP_SIMPLE_PAIR_SCORE(NormalizedSphereDistancePairScore,
                        get_module_version_info());
};


//! A score on a weighted distance between the surfaces of two spheres
/** The distance between the surfaces of the two spheres is multiplied by
    the sum of a specific attribute
    \see SphereDistancePairScore
    \see XYZR
    \see DistancePairScore
 */
class IMPCOREEXPORT WeightedSphereDistancePairScore : public PairScore
{
  IMP::internal::OwnerPointer<UnaryFunction> f_;
  FloatKey radius_;
  FloatKey weight_;
public:
  WeightedSphereDistancePairScore(UnaryFunction *f,
               FloatKey weight,
               FloatKey radius=FloatKey("radius"));
  IMP_SIMPLE_PAIR_SCORE(WeightedSphereDistancePairScore,
                        get_module_version_info());
};


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_SPHERE_DISTANCE_PAIR_SCORE_H */
