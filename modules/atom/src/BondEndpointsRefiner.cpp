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

bool BondEndpointsRefiner::get_can_refine(Particle *p) const {
  return atom::Bond::particle_is_instance(p);
}

Particle *BondEndpointsRefiner::get_refined(Particle *p, unsigned int i) const {
  IMP_INTERNAL_CHECK(get_can_refine(p), "Trying to refine the unrefinable");
  Bond d(p);
  return d.get_bonded(i).get_particle();
}

unsigned int BondEndpointsRefiner::get_number_of_refined(Particle *) const {
  return 2;
}

const ParticlesTemp BondEndpointsRefiner::get_refined(Particle *p) const {
  IMP_INTERNAL_CHECK(get_can_refine(p), "Trying to refine the unrefinable");
  Bond d(p);
  ParticlesTemp ps(2);
  ps[0] = d.get_bonded(0);
  ps[1] = d.get_bonded(1);
  return ps;
}

ParticlesTemp BondEndpointsRefiner::get_input_particles(Particle *p) const {
  ParticlesTemp ret = get_refined(p);
  ret.push_back(p);
  return ret;
}

ContainersTemp BondEndpointsRefiner::get_input_containers(Particle *) const {
  return ContainersTemp();
}

void BondEndpointsRefiner::do_show(std::ostream &) const {}

IMPATOM_END_NAMESPACE
