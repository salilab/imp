/**
 *  \file CenterOfMass.cpp
 *  \brief Center of mass of a set of particles
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/atom/CenterOfMass.h"

#include <IMP/decorator_macros.h>
#include <IMP/atom/Mass.h>
#include <IMP/core/CentroidOfRefined.h>
#include <IMP/core/FixedRefiner.h>
#include <IMP/core/SingletonConstraint.h>
#include <IMP/core/XYZ.h>
#include <IMP/core/WeightedDerivativesToRefined.h>



IMPATOM_BEGIN_NAMESPACE


void CenterOfMass::do_setup_particle(kernel::Model *m,
                                       ParticleIndex pi,
                                       const ParticleIndexes &pis)
{
  Refiner *refiner = new core::FixedRefiner(IMP::kernel::get_particles(m, pis));
  do_setup_particle(m, pi, refiner);
}


void CenterOfMass::do_setup_particle(kernel::Model *m, ParticleIndex pi,
                                     Refiner *refiner) {
  SingletonModifier* pre_mod =
    new core::CentroidOfRefined( refiner,
                           Mass::get_mass_key(),
                           IMP::core::XYZ::get_xyz_keys());
  SingletonDerivativeModifier* post_mod =
    new core::WeightedDerivativesToRefined( refiner,
                                      Mass::get_mass_key(),
                                      IMP::core::XYZ::get_xyz_keys());
  if (!core::XYZ::get_is_setup(m, pi))
    core::XYZ::setup_particle(m, pi);
  if (!Mass::get_is_setup(m, pi))
    Mass::setup_particle(m, pi, 0.0);
  pre_mod->apply_index(m, pi); // update with current state
  set_constraint(pre_mod, post_mod, m, pi);
}


void CenterOfMass::show(std::ostream &out) const {
  core::XYZ xyz(get_particle());
  atom::Mass m(get_particle());
  out << "CenterOfMass at " << xyz
      << " with mass " << m.get_mass();
}


//   ObjectKey CenterOfMass::get_constraint_key() {
//     static ObjectKey ret("CenterOfMass score state");
//     return ret;
//   }
//   void CenterOfMass::set_constraint(SingletonModifier* before,
//                             SingletonDerivativeModifier* after, Model* m,
//                             ParticleIndex pi) {
//     if (!after && !before) {
//       if (m->get_has_attribute(get_constraint_key(), pi)) {
//         m->remove_score_state(dynamic_cast<ScoreState*>(
//             m->get_attribute(get_constraint_key(), pi)));
//         m->remove_attribute(get_constraint_key(), pi);
//       }
//     } else {
//       Constraint* ss = new core::SingletonConstraint(
//           before, after, m, pi,
//           std::string("CenterOfMass updater for ") + m->get_particle_name(pi));
//       m->add_attribute(get_constraint_key(), pi, ss);
//       m->add_score_state(ss);
//     }
//   }
// IMP_REQUIRE_SEMICOLON_NAMESPACE;

IMP_CONSTRAINT_DECORATOR_DEF(CenterOfMass);


IMPATOM_END_NAMESPACE
