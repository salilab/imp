/**
 *  \file RadiusOfGyrationRestraint.h    \brief radius of gyration restraint.
 *
 *  Restrict max distance between every pair of particle
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_RADIUS_OF_GYRATION_RESTRAINT_H
#define IMPMULTIFIT_RADIUS_OF_GYRATION_RESTRAINT_H

#include "multifit_config.h"
#include <IMP/Restraint.h>
#include <IMP/core/DistancePairScore.h>

IMPMULTIFIT_BEGIN_NAMESPACE

//! Ensure a maximum distance between particles
/** defined as maximum distance restraints between all pairs
 */
class IMPMULTIFITEXPORT RadiusOfGyrationRestraint : public Restraint
{
public:
  RadiusOfGyrationRestraint(Particles ps,Float max_radius);
  IMP_RESTRAINT(RadiusOfGyrationRestraint);
  IMP_LIST(private, Particle, particle, Particle*, Particles);
  //IMP_OBJECT_INLINE(RadiusOfGyrationRestraint, show(),release(););
 protected:
  Model *mdl_;
  Float max_radius_;
  core::DistancePairScore* dps_;
};

IMPMULTIFIT_END_NAMESPACE

#endif /* IMPMULTIFIT_RADIUS_OF_GYRATION_RESTRAINT_H */
