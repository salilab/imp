/**
 *  \file IMP/saxs/Restraint.h
 *  \brief Calculate score based on fit to SAXS profile.
 *
 *  Copyright 2007-2023 IMP Inventors. All rights reserved.
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
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>

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
                CA_ATOMS - residue level, residue represented by CA atom
                RESIDUES - residue level, residue represented by bead
  */
  Restraint(const Particles& particles, const Profile* exp_profile,
            FormFactorType ff_type = HEAVY_ATOMS);

  Restraint() {}

  virtual double unprotected_evaluate(IMP::DerivativeAccumulator* accum)
      const override;

  virtual IMP::ModelObjectsTemp do_get_inputs() const override;

  //! \return Information for writing to RMF files
  RestraintInfo *get_static_info() const override;

  IMP_OBJECT_METHODS(Restraint);

 protected:
  ParticleIndexes particles_;
  Pointer<RigidBodiesProfileHandler> handler_;
  Pointer<ProfileFitter<ChiScore> > profile_fitter_;  // computes profiles
  // computes derivatives
  Pointer<DerivativeCalculator> derivative_calculator_;
 private:
  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<IMP::Restraint>(this));
    if (std::is_base_of<cereal::detail::OutputArchiveBase, Archive>::value) {
      Pointer<Profile> exp_profile = const_cast<Profile*>(
                                         profile_fitter_->get_profile());
      ar(handler_->get_form_factor_type(), particles_, exp_profile);
    } else {
      FormFactorType ff_type;
      Pointer<Profile> exp_profile;
      ar(ff_type, particles_, exp_profile);
      handler_ = new RigidBodiesProfileHandler(
                      IMP::get_particles(get_model(), particles_), ff_type);
      profile_fitter_ = new ProfileFitter<ChiScore>(exp_profile);
      derivative_calculator_ = new DerivativeCalculator(exp_profile);
    }
  }
  IMP_OBJECT_SERIALIZE_DECL(Restraint);
};

IMPSAXS_END_NAMESPACE

#endif /* IMPSAXS_RESTRAINT_H */
