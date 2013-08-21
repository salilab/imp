/**
 *  \file IMP/saxs/Restraint.h
 *  \brief Calculate score based on fit to SAXS profile.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSAXS_RESTRAINT_H
#define IMPSAXS_RESTRAINT_H

#include <IMP/saxs/saxs_config.h>

#include <IMP/saxs/ProfileFitter.h>
#include <IMP/saxs/Profile.h>
#include <IMP/saxs/DerivativeCalculator.h>

#include <IMP/core/rigid_bodies.h>

#include <IMP/Model.h>
#include <IMP/Restraint.h>
#include <IMP/VersionInfo.h>
#include <IMP/base/Pointer.h>

IMPSAXS_BEGIN_NAMESPACE

//! Calculate score based on fit to SAXS profile
/** \ingroup exp_restraint

    The restraint takes rigid bodies into account, in order
    to speed up the calculations. Rigid body should be gived as single
    RigidBody Particle. Other, non-rigid body Particles can also be given.

    The shape of a rigid body is assumed to be defined by the set of
    atom::Hierarchy leaves of the atom::Hierarchy rooted at the rigid body
    particle.

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
     \param[in] ff_type Type of the form factors for profile calculations:
                ALL_ATOMS - all atoms including hydrogens
                HEAVY_ATOMS - no hydrogens, all other atoms included
                CA_ATOMS - residue level, residue represented by CA
  */
  Restraint(const Particles& particles, const Profile* exp_profile,
            FormFactorType ff_type = HEAVY_ATOMS);

  virtual double
  unprotected_evaluate(IMP::kernel::DerivativeAccumulator *accum)
     const IMP_OVERRIDE;
  virtual IMP::kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(Restraint);

 protected:
  void compute_profile(Profile* model_profile);

 protected:
  Particles particles_; // non-rigid bodies particles
  std::vector<core::RigidBody> rigid_bodies_decorators_; //rigid bodies
  std::vector<Particles> rigid_bodies_; // rigid bodies particles
  Profile *rigid_bodies_profile_; // non-changing part of the profile
  base::Pointer<ProfileFitter<ChiScore> > profile_fitter_; // computes profiles
  // computes derivatives
  base::Pointer<DerivativeCalculator> derivative_calculator_;
  FormFactorType ff_type_; // type of the form factors to use
};

IMPSAXS_END_NAMESPACE

#endif  /* IMPSAXS_RESTRAINT_H */
