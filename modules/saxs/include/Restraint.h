/**
 *  \file Restraint.h
 *  \brief Calculate score based on fit to SAXS profile.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPSAXS_RESTRAINT_H
#define IMPSAXS_RESTRAINT_H

#include "config.h"
#include "internal/version_info.h"

#include <IMP/saxs/Score.h>
#include <IMP/saxs/Profile.h>

#include <IMP/core/rigid_bodies.h>

#include <IMP/Model.h>
#include <IMP/Restraint.h>
#include <IMP/VersionInfo.h>

IMPSAXS_BEGIN_NAMESPACE

//! Calculate score based on fit to SAXS profile
/** \ingroup exp_restraint

    The restraint takes rigid bodies into account, in order
    to speed up the calculations. Rigid body should be gived as single
    RigidBody Particle. Other, non-rigid body Particles can also be given.

    \par Algorithmic details:
    The distances between the atoms of rigid body do not change, therefore
    their contribution to the profile is pre-computed and stored.
 */
class IMPSAXSEXPORT Restraint : public IMP::Restraint
{
 public:
  //! Constructor
  /**
     \param[in] particles The particles participating in the fitting score
     \param[in] exp_profile  The experimental profile used in the fitting score
     \param[in] ff_table Form Factor Table
  */
  Restraint(const Particles& particles, const Profile& exp_profile,
            FormFactorTable* ff_table);

  IMP_RESTRAINT(Restraint, internal::version_info)

  //only needed in domino
  ParticlesList get_interacting_particles() const
  {
    return ParticlesList(1, Particles(particles_.begin(), particles_.end()));
  }

 private:
  void compute_profile(Profile& model_profile);
 private:
  FormFactorTable* ff_table_; // pointer to form factors table
  Particles particles_; // non-rigid bodies particles
  std::vector<core::RigidBody> rigid_bodies_decorators_; //rigid bodies
  std::vector<Particles> rigid_bodies_; // rigid bodies particles
  Profile rigid_bodies_profile_; // non-changing part of the profile
  Score *saxs_score_; // computes profiles and derivatives
};

IMPSAXS_END_NAMESPACE

#endif  /* IMPSAXS_RESTRAINT_H */
