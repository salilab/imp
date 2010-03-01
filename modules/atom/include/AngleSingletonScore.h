/**
 *  \file atom/AngleSingletonScore.h
 *  \brief A score on the deviation of an angle from ideality.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#ifndef IMPATOM_ANGLE_SINGLETON_SCORE_H
#define IMPATOM_ANGLE_SINGLETON_SCORE_H

#include "config.h"
#include "bond_decorators.h"
#include <IMP/SingletonScore.h>
#include <IMP/UnaryFunction.h>
#include <IMP/Pointer.h>

IMPATOM_BEGIN_NAMESPACE

//! Score the angle based on a UnaryFunction,
/** The score is based on the difference between the stored ideal
    angle and the actual angle and scaled by the stiffness. That is
    stiffness * (angle-ideal_value). The difference is in radians
    between -pi and +pi; it is the shortest distance from one angle to the
    other.
 */
class IMPATOMEXPORT AngleSingletonScore : public SingletonScore
{
  IMP::internal::OwnerPointer<UnaryFunction> f_;
public:
  //! Use f to penalize deviations in angle
  AngleSingletonScore(UnaryFunction *f);
  IMP_SINGLETON_SCORE(AngleSingletonScore);
};

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_ANGLE_SINGLETON_SCORE_H */
