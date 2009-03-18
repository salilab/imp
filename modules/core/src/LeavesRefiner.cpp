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
  return core::HierarchyDecorator(p, traits_).get_number_of_children() != 0;

}

Particles LeavesRefiner::get_refined(Particle *p) const
{
  IMP_assert(get_can_refine(p), "Trying to refine the unrefinable");
  core::HierarchyDecorator d(p, traits_);
  return get_leaves(d);
}



void LeavesRefiner::cleanup_refined(Particle *,
                                              Particles &,
                                              DerivativeAccumulator *) const
{
  // This space left intentionally blank
}

void LeavesRefiner::show(std::ostream &out) const
{
  out << "LeavesRefiner" << std::endl;
}

IMPCORE_END_NAMESPACE
