/**
 *  \file LeavesRefiner.cpp
 *  \brief Return the hierarchy children of a particle.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#include <IMP/core/LeavesRefiner.h>

#include <IMP/core/Hierarchy.h>

IMPCORE_BEGIN_NAMESPACE

LeavesRefiner
::LeavesRefiner(HierarchyTraits traits):
  Refiner("LeavesRefiner%d"), traits_(traits)
{
}


bool LeavesRefiner::get_can_refine(Particle *p) const
{
  return core::Hierarchy::particle_is_instance(p, traits_);

}

Particle* LeavesRefiner::get_refined(Particle *p, unsigned int i) const
{
  return core::Hierarchy(p, traits_).get_leaves()[i];
}

unsigned int LeavesRefiner::get_number_of_refined(Particle *p) const
{
  return core::Hierarchy(p, traits_).get_leaves().size();
}



const ParticlesTemp LeavesRefiner::get_refined(Particle *p) const{
  // force filling of the cache, yeah, its not good organization
  IMP_INTERNAL_CHECK(get_can_refine(p), "Trying to refine the unrefinable");
  return core::Hierarchy(p, traits_).get_leaves();
}

ParticlesTemp LeavesRefiner::get_input_particles(Particle *p) const {
  return ParticlesTemp();
}

ContainersTemp LeavesRefiner::get_input_containers(Particle *p) const {
  GenericHierarchiesTemp t=get_all_descendants(Hierarchy(p, traits_));
  return ContainersTemp(t.begin(), t.end());
}


void LeavesRefiner::do_show(std::ostream &out) const
{
}

IMPCORE_END_NAMESPACE
