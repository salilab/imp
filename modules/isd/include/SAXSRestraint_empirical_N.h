/**
 *  \file saxs/Restraint.h
 *  \brief Calculate score based on fit to SAXS profile.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_SAXS_RESTRAINT_EMPIRICAL_N_H
#define IMPISD_SAXS_RESTRAINT_EMPIRICAL_N_H

#include "isd_config.h"

#include <IMP/saxs/Profile.h>

#include <IMP/core/rigid_bodies.h>

#include <IMP/isd/Scale.h>

#include <IMP/Model.h>
#include <IMP/isd/ISDRestraint.h>
#include <IMP/VersionInfo.h>

IMPISD_BEGIN_NAMESPACE

//! Calculate score based on fit to SAXS profile. Normal model with one
//  weight. Empirical Bayes.
/** \ingroup exp_restraint

    The likelihood of this model is a normal. Assume M data points.
    \f[p(D | X,\sigma,\gamma, I) = \frac{1}{(\sqrt{2\pi}\sigma)^M} 
    \prod_{q=q_{min}}^{q_{max}} \frac{1}{\sigma_{exp}(q)}
    \exp left[ - \frac{1}{2\sigma^2\sigma^2_{exp}(q)} 
                \left(I_{exp}(q) - \gamma I_{calc}(q)
                \right)^2 \right] \f]

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
class IMPISDEXPORT SAXSRestraint_empirical_N : public ISDRestraint
{
 public:
  //! Constructor
  /**
     \param[in] particles The particles that generated the profile, usually atoms of a protein.
     \param[in] sigma An ISD Scale particle that will be \f$\sigma\f$
     \param[in] gamma An ISD Scale particle that will set the scaling factor \f$\gamma\f$ 
                between experiment and mock data.
     \param[in] exp_profile  The experimental profile used to restrain the structure.
     \param[in] ff_type Type of the form factors for profile calculations:
                ALL_ATOMS - all atoms including hydrogens
                HEAVY_ATOMS - no hydrogens, all other atoms included
                CA_ATOMS - residue level, residue represented by CA
  */
  SAXSRestraint_empirical_N(const Particles& particles, const Scale& sigma, const
          Scale& gamma, const saxs::Profile& exp_profile, saxs::FormFactorType ff_type = saxs::HEAVY_ATOMS);

  IMP_RESTRAINT(SAXSRestraint_empirical_N);

  double get_probability() const { return std::exp(-unprotected_evaluate(NULL));}

 private:
  void compute_profile(saxs::Profile& model_profile);
  Particles particles_; // non-rigid bodies particles
  Scale sigma_; // global weight
  Scale gamma_; // calibration factor
  std::vector<core::RigidBody> rigid_bodies_decorators_; //rigid bodies
  std::vector<Particles> rigid_bodies_; // rigid bodies particles
  saxs::Profile rigid_bodies_profile_; // non-changing part of the profile
  saxs::Profile exp_profile_; // experimental profile
  saxs::FormFactorType ff_type_; // type of the form factors to use
};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_SAXS_RESTRAINT_EMPIRICAL_N_H */
