/**
 *  \file IMP/multifit/RigidLeavesRefiner.cpp
 *  \brief Return all rigid body members that are also hierarchy leaves.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/multifit/RigidLeavesRefiner.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/atom/Hierarchy.h>

IMPMULTIFIT_BEGIN_NAMESPACE

bool RigidLeavesRefiner::get_can_refine(Particle *p) const {
  return core::RigidBody::get_is_setup(p);
}

const ParticlesTemp RigidLeavesRefiner::get_refined(Particle *p) const {
  ParticlesTemp members = core::RigidBody(p).get_rigid_members();
  ParticlesTemp ret;
  for (ParticlesTemp::const_iterator it = members.begin(); it != members.end();
       ++it) {
    if (atom::Hierarchy::get_is_setup(*it)
        && atom::Hierarchy(*it).get_number_of_children() == 0) {
      ret.push_back(*it);
    }
  }
  return ret;
}

ModelObjectsTemp RigidLeavesRefiner::do_get_inputs(
    Model *m, const ParticleIndexes &pis) const {
  return IMP::get_particles(m, pis);
}

IMPMULTIFIT_END_NAMESPACE
