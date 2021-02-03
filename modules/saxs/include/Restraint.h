/**
 *  \file IMP/saxs/Restraint.h
 *  \brief Calculate score based on fit to SAXS profile.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSAXS_RESTRAINT_H
#define IMPSAXS_RESTRAINT_H

#include <IMP/saxs/saxs_config.h>

#include <IMP/saxs/ProfileFitter.h>
#include <IMP/saxs/Profile.h>
#include <IMP/saxs/DerivativeCalculator.h>

#include <IMP/saxs/RigidBodiesProfileHandler.h>

#include <IMP/Model.h>
#include <IMP/Restraint.h>
#include <IMP/Object.h>
#include <IMP/Pointer.h>

IMPSAXS_BEGIN_NAMESPACE

//! Calculate score based on fit to SAXS profile
/** \ingroup exp_restraint

    The restraint takes rigid bodies into account, in order
    to speed up the calculations. Rigid body should be given as single
    RigidBody Particle. Other, non-rigid body Particles can also be given.

    The shape of a rigid body is assumed to be defined by the set of
    atom::Hierarchy leaves of the atom::Hierarchy rooted at the rigid body
    particle.

    \par Algorithmic details:
    The distances between the atoms of rigid body do not change, therefore
    their contribution to the profile is pre-computed and stored.
 */
class IMPSAXSEXPORT Restraint : public IMP::Restraint {
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

  virtual double unprotected_evaluate(IMP::DerivativeAccumulator* accum)
      const IMP_OVERRIDE;

  virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;

  //! \return Information for writing to RMF files
  RestraintInfo *get_static_info() const IMP_OVERRIDE;

  IMP_OBJECT_METHODS(Restraint);

 protected:
  FormFactorType ff_type_;
  Pointer<RigidBodiesProfileHandler> handler_;
  Pointer<ProfileFitter<ChiScore> > profile_fitter_;  // computes profiles
  // computes derivatives
  Pointer<DerivativeCalculator> derivative_calculator_;
};

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_RESTRAINT_H */
