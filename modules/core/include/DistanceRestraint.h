/**
 *  \file DistanceRestraint.h   \brief Distance restraint between two particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_DISTANCE_RESTRAINT_H
#define IMPCORE_DISTANCE_RESTRAINT_H

#include "config.h"
#include "internal/version_info.h"
#include "DistancePairScore.h"

#include <IMP/Restraint.h>

#include <iostream>

IMPCORE_BEGIN_NAMESPACE

//! Distance restraint between two particles
/**
   \note If the particles are closer than a certain distance, then
   the contributions to the derivatives are set to 0.
 */
class IMPCOREEXPORT DistanceRestraint : public Restraint
{
public:
  //! Create the distance restraint.
  /** \param[in] score_func Scoring function for the restraint.
      \param[in] p1 Pointer to first particle in distance restraint.
      \param[in] p2 Pointer to second particle in distance restraint.
   */
  DistanceRestraint(UnaryFunction* score_func,
                    Particle* p1, Particle* p2);
  virtual ~DistanceRestraint() {}

  ParticlesList get_interacting_particles() const {
    ParticlesList ret(1,Particles(p_, p_+2));
    return ret;
  }

  IMP_RESTRAINT(internal::version_info)

private:
  DistancePairScore dp_;
  Pointer<Particle> p_[2];
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_DISTANCE_RESTRAINT_H */
