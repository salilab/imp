/**
 *  \file IMP/core/AngleRestraint.h
 *  \brief Angle restraint between three particles.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
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
  /** \param[in] m Model.
      \param[in] score_func Scoring function for the restraint.
      \param[in] p1 First particle in angle restraint.
      \param[in] p2 Second particle in angle restraint.
      \param[in] p3 Third particle in angle restraint.
   */
  AngleRestraint(Model *m, UnaryFunction* score_func, ParticleIndexAdaptor p1,
                 ParticleIndexAdaptor p2, ParticleIndexAdaptor p3);

  IMP_OBJECT_METHODS(AngleRestraint);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_ANGLE_RESTRAINT_H */
