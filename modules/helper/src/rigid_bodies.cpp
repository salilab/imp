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
#include <IMP/core/LeavesRefiner.h>
#include <IMP/atom/Hierarchy.h>
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
  IMP_USAGE_CHECK(rbs->get_number_of_particles() >0,
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

namespace {
  struct IsXYZ {
    template <class H>
    bool operator()(H h) const {
      return core::XYZ::particle_is_instance(h);
    }
  };
}

ScoreState* create_rigid_body(atom::Hierarchy h) {
  Particles eps;
  core::gather(h, core::XYZ::particle_is_instance, std::back_inserter(eps));
  core::XYZs extra_members(eps);
  if (!core::XYZ::particle_is_instance(h)) {
    core::XYZR d= core::XYZR::setup_particle(h);
  }
  core::RigidBody rbd
    = core::RigidBody::setup_particle(h, extra_members);
  SMP sm= get_modifiers(false);
  rbd.set_coordinates_are_optimized(true, false);
  core::SingletonScoreState *sss
    = new core::SingletonScoreState(sm.first, sm.second, h);
  cover_rigid_body(rbd, new core::LeavesRefiner(h.get_traits()));
  return sss;
}



void cover_rigid_body(core::RigidBody d, Refiner *ref, FloatKey rk) {
  double md=0;
  // make sure it gets cleaned up properly
  Pointer<Refiner> rp(ref);
  for (unsigned int i=0; i< ref->get_number_of_refined(d); ++i) {
    core::RigidMember rm(ref->get_refined(d,i));
    double cd= rm.get_internal_coordinates().get_magnitude();
    if (rm.get_particle()->has_attribute(rk)) {
      cd+= rm.get_particle()->get_value(rk);
    }
    md=std::max(cd, md);
  }
  if (d.get_particle()->has_attribute(rk)) {
    d.get_particle()->set_value(rk, md);
  } else {
    d.get_particle()->add_attribute(rk, md);
  }
}

IMPHELPER_END_NAMESPACE
