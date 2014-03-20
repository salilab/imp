/**
 *  \file ChildrenRefiner.cpp
 *  \brief Return the hierarchy children of a particle.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#include <IMP/core/ChildrenRefiner.h>
#include <IMP/container_base.h>
#include <IMP/core/Hierarchy.h>

IMPCORE_BEGIN_NAMESPACE

ChildrenRefiner::ChildrenRefiner(HierarchyTraits traits)
    : Refiner("Children%d"), traits_(traits) {}

bool ChildrenRefiner::get_can_refine(kernel::Particle *p) const {
  if (!core::Hierarchy::get_is_setup(p, traits_)) return false;
  return core::Hierarchy(p, traits_).get_number_of_children() != 0;
}

const kernel::ParticlesTemp ChildrenRefiner::get_refined(kernel::Particle *p)
    const {
  Hierarchy hd(p, traits_);
  kernel::ParticlesTemp ret(hd.get_number_of_children());
  for (unsigned int i = 0; i < ret.size(); ++i) {
    ret[i] = hd.get_child(i);
  }
  return ret;
}

ModelObjectsTemp ChildrenRefiner::do_get_inputs(
    kernel::Model *m, const kernel::ParticleIndexes &pis) const {
  return IMP::kernel::get_particles(m, pis);
}

IMPCORE_END_NAMESPACE
