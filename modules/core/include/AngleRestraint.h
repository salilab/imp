/**
 *  \file IMP/core/AngleRestraint.h
 *  \brief Angle restraint between three particles.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_ANGLE_RESTRAINT_H
#define IMPCORE_ANGLE_RESTRAINT_H

#include <IMP/core/core_config.h>
#include "AngleTripletScore.h"
#include "XYZ.h"

#include "TripletRestraint.h"
#include <IMP/UnaryFunction.h>

IMPCORE_BEGIN_NAMESPACE

//! Angle restraint between three particles
/** \see AngleTripletScore
 */
class IMPCOREEXPORT AngleRestraint : public TripletRestraint {
 public:
  //! Create the angle restraint.
  /** \param[in] score_func Scoring function for the restraint.
      \param[in] p1 Pointer to first particle in angle restraint.
      \param[in] p2 Pointer to second particle in angle restraint.
      \param[in] p3 Pointer to third particle in angle restraint.
   */
  AngleRestraint(UnaryFunction* score_func, kernel::Particle* p1,
                 kernel::Particle* p2, kernel::Particle* p3);

  AngleRestraint(UnaryFunction* score_func, XYZ p0, XYZ p1, XYZ p2);

  IMP_OBJECT_METHODS(AngleRestraint);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_ANGLE_RESTRAINT_H */
