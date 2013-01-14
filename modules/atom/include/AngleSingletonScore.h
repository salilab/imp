/**
 *  \file IMP/atom/AngleSingletonScore.h
 *  \brief A score on the deviation of an angle from ideality.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_ANGLE_SINGLETON_SCORE_H
#define IMPATOM_ANGLE_SINGLETON_SCORE_H

#include <IMP/atom/atom_config.h>
#include "bond_decorators.h"
#include <IMP/SingletonScore.h>
#include <IMP/singleton_macros.h>
#include <IMP/UnaryFunction.h>
#include <IMP/Pointer.h>

IMPATOM_BEGIN_NAMESPACE

//! Score the angle based on a UnaryFunction,
/** This scores the angle using information stored in its Angle decorator.
    The score is based on the difference between the stored ideal
    angle and the actual angle and scaled by the stiffness. That is
    stiffness * (angle-ideal_value). The difference is in radians
    between -pi and +pi; it is the shortest distance from one angle to the
    other.

    \see CHARMMParameters::create_angles(), Angle.
 */
class IMPATOMEXPORT AngleSingletonScore : public SingletonScore
{
  IMP::OwnerPointer<UnaryFunction> f_;
public:
  //! Use f to penalize deviations in angle
  AngleSingletonScore(UnaryFunction *f);
  IMP_SINGLETON_SCORE(AngleSingletonScore);
};

IMP_OBJECTS(AngleSingletonScore,AngleSingletonScores);

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_ANGLE_SINGLETON_SCORE_H */
