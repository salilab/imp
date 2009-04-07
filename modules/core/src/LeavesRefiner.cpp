/**
 *  \file LeavesRefiner.cpp
 *  \brief Return the hierarchy children of a particle.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#include <IMP/core/LeavesRefiner.h>

#include <IMP/core/HierarchyDecorator.h>

IMPCORE_BEGIN_NAMESPACE

LeavesRefiner
::LeavesRefiner(HierarchyTraits traits): traits_(traits)
{
}


bool LeavesRefiner::get_can_refine(Particle *p) const
{
  if (!core::HierarchyDecorator::is_instance_of(p, traits_)) return false;
  if (core::HierarchyDecorator(p, traits_).get_number_of_children() != 0) {
    cache_[p]=get_leaves(core::HierarchyDecorator(p, traits_));
    return true;
  } else {
    return false;
  }

}

Particle* LeavesRefiner::get_refined(Particle *p, unsigned int i) const
{
  // force filling of the cache, yeah, its not good organization
  get_can_refine(p);
  IMP_assert(get_can_refine(p), "Trying to refine the unrefinable");
  return cache_[p][i];
}

unsigned int LeavesRefiner::get_number_of_refined(Particle *p) const
{
  // force filling of the cache, yeah, its not good organization
  get_can_refine(p);
  IMP_assert(get_can_refine(p), "Trying to refine the unrefinable");
  return cache_[p].size();
}



const Particles LeavesRefiner::get_refined(Particle *p) const{
  // force filling of the cache, yeah, its not good organization
  get_can_refine(p);
  IMP_assert(get_can_refine(p), "Trying to refine the unrefinable");
  return cache_[p];
}


void LeavesRefiner::show(std::ostream &out) const
{
  out << "LeavesRefiner" << std::endl;
}

IMPCORE_END_NAMESPACE
