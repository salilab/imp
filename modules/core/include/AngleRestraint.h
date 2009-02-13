/**
 *  \file AngleRestraint.h   \brief Angle restraint between three particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_ANGLE_RESTRAINT_H
#define IMPCORE_ANGLE_RESTRAINT_H

#include "config.h"
#include "internal/version_info.h"
#include "AngleTripletScore.h"

#include <IMP/Restraint.h>
#include <IMP/Pointer.h>
#include <IMP/UnaryFunction.h>

IMPCORE_BEGIN_NAMESPACE

//! Angle restraint between three particles
class IMPCOREEXPORT AngleRestraint : public Restraint
{
public:
  //! Create the angle restraint.
  /** \param[in] score_func Scoring function for the restraint.
      \param[in] p1 Pointer to first particle in angle restraint.
      \param[in] p2 Pointer to second particle in angle restraint.
      \param[in] p3 Pointer to third particle in angle restraint.
   */
  AngleRestraint(UnaryFunction* score_func,
                 Particle* p1, Particle* p2, Particle* p3);
  virtual ~AngleRestraint(){}

  IMP_RESTRAINT(internal::version_info)

  ParticlesList get_interacting_particles() const
  {
    return ParticlesList(1, Particles(p_, p_+3));
  }
protected:
  Pointer<AngleTripletScore> sf_;
  Pointer<Particle> p_[3];
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_ANGLE_RESTRAINT_H */
