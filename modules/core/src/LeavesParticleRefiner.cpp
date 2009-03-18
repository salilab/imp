/**
 *  \file LeavesParticleRefiner.cpp
 *  \brief Return the hierarchy children of a particle.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#include <IMP/core/LeavesParticleRefiner.h>

#include <IMP/core/HierarchyDecorator.h>

IMPCORE_BEGIN_NAMESPACE

LeavesParticleRefiner
::LeavesParticleRefiner(HierarchyTraits traits): traits_(traits)
{
}


bool LeavesParticleRefiner::get_can_refine(Particle *p) const
{
  if (!core::HierarchyDecorator::is_instance_of(p, traits_)) return false;
  return core::HierarchyDecorator(p, traits_).get_number_of_children() != 0;

}

Particles LeavesParticleRefiner::get_refined(Particle *p) const
{
  IMP_assert(get_can_refine(p), "Trying to refine the unrefinable");
  core::HierarchyDecorator d(p, traits_);
  return get_leaves(d);
}



void LeavesParticleRefiner::cleanup_refined(Particle *,
                                              Particles &,
                                              DerivativeAccumulator *) const
{
  // This space left intentionally blank
}

void LeavesParticleRefiner::show(std::ostream &out) const
{
  out << "LeavesParticleRefiner" << std::endl;
}

IMPCORE_END_NAMESPACE
