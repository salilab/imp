/**
 *  \file saxs/Restraint.h
 *  \brief Calculate score based on fit to SAXS profile.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_SAXS_RESTRAINT_SLIDING_LN_H
#define IMPISD_SAXS_RESTRAINT_SLIDING_LN_H

#include "isd_config.h"

#include <IMP/saxs/Profile.h>

#include <IMP/core/rigid_bodies.h>

#include <IMP/isd/Scale.h>

#include <IMP/Model.h>
#include <IMP/isd/ISDRestraint.h>
#include <IMP/VersionInfo.h>

IMPISD_BEGIN_NAMESPACE

//! Calculate score based on fit to SAXS profile. Lognormal model with one
//  weight. Restraint weights differently parts where \f$q\le q_0\f$ and \f$q>q_0\f$
/** \ingroup exp_restraint

    The likelihood of this model is a lognormal. Assume M data points.
    \f[p(D | X,\sigma,\gamma, I) = \frac{1}{(\sqrt{2\pi}\sigma)^M} 
    \prod_{q=q_\text{min}}^{q_\text{max}} \frac{1}{\sigma(q) I_\text{exp}(q)}
    \exp left[ - \frac{1}{2\sigma^2(q)} 
                \log^2 \left(\frac{I_{exp}(q)}{\gamma I_{calc}(q)}
                \right) \right] \f]
    where
    \f[\sigma^2(q) = \frac{q_0 \alpha_\text{Good}}{\sigma^2_\text{exp}(q)}\f]
    when \f$q\le q_0\f$ and 
    \f[\sigma^2(q) = \frac{q \alpha_\text{Bad}}{\sigma^2_\text{exp}(q)}\f]
    when \f$q>q_0\f$.
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
class IMPISDEXPORT SAXSRestraint_sliding_LN : public ISDRestraint
{
 public:
  //! Constructor
  /**
     \param[in] particles The particles that generated the profile, usually atoms of a protein.
     \param[in] gamma An ISD Scale particle that will set the scaling factor \f$\gamma\f$ 
                between experiment and mock data.
     \param[in] alphag An ISD Scale particle that will be \f$\alpha_\text{Good}\f$
     \param[in] alphab An ISD Scale particle that will be \f$\alpha_\text{Bad}\f$
     \param[in] q0 An ISD Scale particle that will be \f$q_0\f$
     \param[in] exp_profile  The experimental profile used to restrain the structure.
     \param[in] ff_type Type of the form factors for profile calculations:
                ALL_ATOMS - all atoms including hydrogens
                HEAVY_ATOMS - no hydrogens, all other atoms included
                CA_ATOMS - residue level, residue represented by CA
  */
  SAXSRestraint_sliding_LN(const Particles& particles, const Scale& gamma, const Scale& alphag, 
          const Scale& alphab, const Scale& q0,
          const saxs::Profile& exp_profile, saxs::FormFactorType ff_type = saxs::HEAVY_ATOMS);

  IMP_RESTRAINT(SAXSRestraint_sliding_LN);

  double get_probability() const { return std::exp(-unprotected_evaluate(NULL));}

 private:
  void compute_profile(saxs::Profile& model_profile);
  Particles particles_; // non-rigid bodies particles
  Scale gamma_; // calibration factor
  Scale alphag_; // weight for good portion
  Scale alphab_; // weight for bad portion
  Scale q0_; // separation between good and bad region
  std::vector<core::RigidBody> rigid_bodies_decorators_; //rigid bodies
  std::vector<Particles> rigid_bodies_; // rigid bodies particles
  saxs::Profile rigid_bodies_profile_; // non-changing part of the profile
  saxs::Profile exp_profile_; // experimental profile
  saxs::FormFactorType ff_type_; // type of the form factors to use
};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_SAXS_RESTRAINT_SLIDING_LN_H */
