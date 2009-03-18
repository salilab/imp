/**
 *  \file BondEndpointsRefiner.cpp
 *  \brief Return the hierarchy children of a particle.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#include <IMP/atom/BondEndpointsRefiner.h>
#include <IMP/atom/bond_decorators.h>

IMPATOM_BEGIN_NAMESPACE

BondEndpointsRefiner::BondEndpointsRefiner()
{
}


bool BondEndpointsRefiner::get_can_refine(Particle *p) const
{
  return atom::BondDecorator::is_instance_of(p);
}

Particles BondEndpointsRefiner::get_refined(Particle *p) const
{
  IMP_assert(get_can_refine(p), "Trying to refine the unrefinable");
  BondDecorator d(p);
  Particles ps(2);
  ps[0]= d.get_bonded(0).get_particle();
  ps[1]= d.get_bonded(1).get_particle();
  return ps;
}



void BondEndpointsRefiner::cleanup_refined(Particle *,
                                              Particles &,
                                              DerivativeAccumulator *) const
{
  // This space left intentionally blank
}

void BondEndpointsRefiner::show(std::ostream &out) const
{
  out << "BondEndpointsRefiner" << std::endl;
}


IMPATOM_END_NAMESPACE
