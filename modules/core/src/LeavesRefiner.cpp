/**
 *  \file LeavesRefiner.cpp
 *  \brief Return the hierarchy children of a particle.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/core/LeavesRefiner.h>

#include <IMP/core/Hierarchy.h>

IMPCORE_BEGIN_NAMESPACE

LeavesRefiner::LeavesRefiner(HierarchyTraits traits)
    : Refiner("LeavesRefiner%d"), traits_(traits) {}

bool LeavesRefiner::get_can_refine(Particle *p) const {
  return core::Hierarchy::particle_is_instance(p, traits_);

}

Particle *LeavesRefiner::get_refined(Particle *p, unsigned int i) const {
  return core::get_leaves(Hierarchy(p, traits_))[i];
}

unsigned int LeavesRefiner::get_number_of_refined(Particle *p) const {
  return core::get_leaves(Hierarchy(p, traits_)).size();
}

const ParticlesTemp LeavesRefiner::get_refined(Particle *p) const {
  // force filling of the cache, yeah, its not good organization
  IMP_INTERNAL_CHECK(get_can_refine(p), "Trying to refine the unrefinable");
  return core::get_leaves(Hierarchy(p, traits_));
}

ParticlesTemp LeavesRefiner::get_input_particles(Particle *p) const {
  GenericHierarchies t = get_all_descendants(Hierarchy(p, traits_));
  return ParticlesTemp(t.begin(), t.end());
}

ContainersTemp LeavesRefiner::get_input_containers(Particle *) const {
  return ContainersTemp();
}

void LeavesRefiner::do_show(std::ostream &) const {}

IMPCORE_END_NAMESPACE
