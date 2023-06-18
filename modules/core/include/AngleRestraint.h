/**
 *  \file IMP/core/AngleRestraint.h
 *  \brief Angle restraint between three particles.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_ANGLE_RESTRAINT_H
#define IMPCORE_ANGLE_RESTRAINT_H

#include <IMP/core/core_config.h>
#include "AngleTripletScore.h"
#include "XYZ.h"

#include <IMP/core/TripletRestraint.h>
#include <IMP/UnaryFunction.h>
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/types/polymorphic.hpp>

IMPCORE_BEGIN_NAMESPACE

//! Angle restraint between three particles
/** \see AngleTripletScore
 */
class IMPCOREEXPORT AngleRestraint : public TripletRestraint {
  friend class cereal::access;

  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<TripletRestraint>(this));
  }

  IMP_OBJECT_SERIALIZE_DECL(AngleRestraint);

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
  AngleRestraint() {}

  IMP_OBJECT_METHODS(AngleRestraint);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_ANGLE_RESTRAINT_H */
