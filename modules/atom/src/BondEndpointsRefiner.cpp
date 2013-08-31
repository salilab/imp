/**
 *  \file BondEndpointsRefiner.cpp
 *  \brief Return the hierarchy children of a particle.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/atom/BondEndpointsRefiner.h>
#include <IMP/atom/bond_decorators.h>

IMPATOM_BEGIN_NAMESPACE

BondEndpointsRefiner::BondEndpointsRefiner() {}

bool BondEndpointsRefiner::get_can_refine(kernel::Particle *p) const {
  return atom::Bond::get_is_setup(p);
}

const kernel::ParticlesTemp BondEndpointsRefiner::get_refined(kernel::Particle *p) const {
  IMP_INTERNAL_CHECK(get_can_refine(p), "Trying to refine the unrefinable");
  Bond d(p);
  kernel::ParticlesTemp ps(2);
  ps[0] = d.get_bonded(0);
  ps[1] = d.get_bonded(1);
  return ps;
}

ModelObjectsTemp BondEndpointsRefiner::do_get_inputs(
    Model *m, const kernel::ParticleIndexes &pis) const {
  return IMP::kernel::get_particles(m, pis);
}

IMPATOM_END_NAMESPACE
