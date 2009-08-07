/**
 *  \file rigid_bodies.cpp
 *  \brief Support for rigid bodies.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/helper/rigid_bodies.h"
#include <IMP/SingletonContainer.h>
#include <IMP/core/FixedRefiner.h>
#include <IMP/core/SingletonsScoreState.h>
#include <IMP/core/SingletonScoreState.h>
IMPHELPER_BEGIN_NAMESPACE


namespace {
  typedef std::pair<SingletonModifier*, SingletonModifier*> SMP;
  SMP
  get_modifiers(bool snapping) {
    if (snapping) {
      return SMP(new core::UpdateRigidBodyOrientation(), NULL);
    } else {
      return SMP(new core::UpdateRigidBodyMembers(),
                 new core::AccumulateRigidBodyDerivatives());
    }
  }
}

ScoreState* create_rigid_bodies(SingletonContainer *rbs,
                                Refiner *pr,
                                bool snapping) {
  IMP_check(rbs->get_number_of_particles() >0,
            "Need some particles to make rigid bodies",
            ValueException);
  for (SingletonContainer::ParticleIterator pit= rbs->particles_begin();
       pit != rbs->particles_end();++pit) {
    core::RigidBody rbd= core::RigidBody::setup_particle(*pit,
                                  core::XYZs(pr->get_refined(*pit)));
    rbd.set_coordinates_are_optimized(true, snapping);
  }
  SMP sm= get_modifiers(snapping);
  core::SingletonsScoreState *sss
    = new core::SingletonsScoreState(rbs, sm.first, sm.second);
  return sss;
}

ScoreState* create_rigid_body(Particle *p,
                              const core::XYZs &ps,
                              bool snapping) {
  core::RigidBody rbd= core::RigidBody::setup_particle(p, ps);
  SMP sm= get_modifiers(snapping);
  rbd.set_coordinates_are_optimized(true, snapping);
  core::SingletonScoreState *sss
    = new core::SingletonScoreState(sm.first, sm.second, p);
  return sss;
}



void cover_members(core::RigidBody d, FloatKey rk) {
  double md=0;
  for (unsigned int i=0; i< d.get_number_of_members(); ++i) {
    double cd= d.get_member(i).get_internal_coordinates().get_magnitude();
    if (d.get_member(i).get_particle()->has_attribute(rk)) {
      cd+= d.get_member(i).get_particle()->get_value(rk);
    }
    md=std::max(cd, md);
  }
  d.get_particle()->add_attribute(rk, md);
}

IMPHELPER_END_NAMESPACE
