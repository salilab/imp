/**
 *  \file saxs/Restraint.h
 *  \brief Calculate score based on fit to SAXS profile.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_SAXS_RESTRAINT_MARGINAL_N_H
#define IMPISD_SAXS_RESTRAINT_MARGINAL_N_H

#include "isd_config.h"

#include <IMP/saxs/Profile.h>

#include <IMP/core/rigid_bodies.h>

#include <IMP/isd/Scale.h>

#include <IMP/Model.h>
#include <IMP/isd/ISDRestraint.h>
#include <IMP/VersionInfo.h>

IMPISD_BEGIN_NAMESPACE

//! Calculate score based on fit to SAXS profile. Marginal of the Normal model
//  with respect to the standard deviation.
/** \ingroup exp_restraint

    For M data points, the posterior of this model is
    \f[p(D | X, I) = 
    \left(s^2W_X\right)^{-\frac{M-1}{2}}
    W_X^{-\frac{1}{2}}
    P\left(\frac{M-1}{2}, \frac{s^2W_X}{2}\right)
    \f]
    \f[
    s^2 W_X = \sum{q=q_{min}}^{q_{max}} w_X(q)
    \left[ \frac{I_{exp}(q)}{I_{calc}(q)} - \hat{\gamma} \right]^2
    \f]
    \f[ \hat{\gamma} = \frac{1}{W_X}
    \sum{q=q_{min}}^{q_{max}} w_X(q) \frac{I_{exp}(q)}{I_{calc}(q)}
    \quad
    W_X = \sum{q=q_{min}}^{q_{max}} w_X(q)
    \quad w_X(q) = \frac{I^2_{calc}(q)}{\sigma^2_{exp}(q)}
    \f]

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
class IMPISDEXPORT SAXSRestraint_marginal_N : public ISDRestraint
{
 public:
  //! Constructor
  /**
     \param[in] particles The particles that generated the profile, usually atoms of a protein.
     \param[in] exp_profile  The experimental profile used to restrain the structure.
     \param[in] ff_type Type of the form factors for profile calculations:
                ALL_ATOMS - all atoms including hydrogens
                HEAVY_ATOMS - no hydrogens, all other atoms included
                CA_ATOMS - residue level, residue represented by CA
  */
  SAXSRestraint_marginal_N(const Particles& particles,
          const saxs::Profile& exp_profile, saxs::FormFactorType ff_type = saxs::HEAVY_ATOMS);

  IMP_RESTRAINT(SAXSRestraint_marginal_N);

  double get_probability() const { return std::exp(-unprotected_evaluate(NULL));}

 private:
  void compute_profile(saxs::Profile& model_profile);
  Particles particles_; // non-rigid bodies particles
  std::vector<core::RigidBody> rigid_bodies_decorators_; //rigid bodies
  std::vector<Particles> rigid_bodies_; // rigid bodies particles
  saxs::Profile rigid_bodies_profile_; // non-changing part of the profile
  saxs::Profile exp_profile_; // experimental profile
  saxs::FormFactorType ff_type_; // type of the form factors to use
};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_SAXS_RESTRAINT_MARGINAL_N_H */
