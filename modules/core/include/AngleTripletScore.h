/**
 *  \file AngleTripletScore.h
 *  \brief A Score on the angle between three of particles.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_ANGLE_TRIPLET_SCORE_H
#define IMPCORE_ANGLE_TRIPLET_SCORE_H

#include "config.h"
#include <IMP/TripletScore.h>
#include <IMP/UnaryFunction.h>
#include <IMP/Pointer.h>

IMPCORE_BEGIN_NAMESPACE

//! Apply a function to the angle between three particles.
/** */
class IMPCOREEXPORT AngleTripletScore : public TripletScore
{
  IMP::internal::OwnerPointer<UnaryFunction> f_;
public:
  //! Score the angle (in radians) using f
  AngleTripletScore(UnaryFunction *f);
  IMP_TRIPLET_SCORE(AngleTripletScore, get_module_version_info());
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_ANGLE_TRIPLET_SCORE_H */
