/**
 *  \file saxs/RadiusOfGyrationRestraint.h
 *  \brief Calculate score based on fit to SAXS profile.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSAXS_RADIUS_OF_GYRATION_RESTRAINT_H
#define IMPSAXS_RADIUS_OF_GYRATION_RESTRAINT_H

#include "saxs_config.h"

#include <IMP/saxs/Profile.h>

#include <IMP/Model.h>
#include <IMP/Restraint.h>
#include <IMP/VersionInfo.h>

IMPSAXS_BEGIN_NAMESPACE

//! Calculate score based on radius of gyration, taken from saxs profile
/** \ingroup exp_restraint

 */
class IMPSAXSEXPORT RadiusOfGyrationRestraint : public IMP::Restraint
{
 public:
  //! Constructor
  /**
     \param[in] particles The particles participating in the fitting score
     \param[in] exp_profile  The experimental profile used in the fitting score
  */
  RadiusOfGyrationRestraint(const Particles& particles,
          const Profile& exp_profile, const double end_q_rg=1.3);

  IMP_RESTRAINT(RadiusOfGyrationRestraint);

 private:
  Particles particles_; // non-rigid bodies particles
  double exp_rg_; //radius of gyration from experimental profile
};

IMPSAXS_END_NAMESPACE

#endif  /* IMPSAXS_RADIUS_OF_GYRATION_RESTRAINT_H */
