/**
 *  \file BondEndpointsParticleRefiner.cpp
 *  \brief Return the hierarchy children of a particle.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include <IMP/core/BondEndpointsParticleRefiner.h>
#include <IMP/core/bond_decorators.h>

IMPCORE_BEGIN_NAMESPACE

BondEndpointsParticleRefiner::BondEndpointsParticleRefiner()
{
}


bool BondEndpointsParticleRefiner::get_can_refine(Particle *p) const
{
  return core::BondDecorator::is_instance_of(p);
}

Particles BondEndpointsParticleRefiner::get_refined(Particle *p) const
{
  IMP_assert(get_can_refine(p), "Trying to refine the unrefinable");
  BondDecorator d(p);
  Particles ps(2);
  ps[0]= d.get_bonded(0).get_particle();
  ps[1]= d.get_bonded(1).get_particle();
  return ps;
}



void BondEndpointsParticleRefiner::cleanup_refined(Particle *,
                                              Particles &,
                                              DerivativeAccumulator *) const
{
  // This space left intentionally blank
}

void BondEndpointsParticleRefiner::show(std::ostream &out) const
{
  out << "BondEndpointsParticleRefiner" << std::endl;
}

IMPCORE_END_NAMESPACE
