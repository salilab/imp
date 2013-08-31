/**
 *  \file TableRefiner.cpp
 *  \brief A particle refiner that uses a table.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/core/TableRefiner.h"

IMPCORE_BEGIN_NAMESPACE

TableRefiner::TableRefiner() : Refiner("TableRefiner%d") {}

void TableRefiner::add_particle(kernel::Particle *p, const kernel::ParticlesTemp &ps) {
  IMP_USAGE_CHECK(map_.find(p) == map_.end(),
                  "Particle " << p->get_name() << " already in map.");
  map_[p] = get_as<kernel::Particles>(ps);
}

void TableRefiner::remove_particle(kernel::Particle *p) {
  IMP_USAGE_CHECK(map_.find(p) != map_.end(),
                  "Particle " << p->get_name() << " not found in map.");
  map_.erase(p);
}

void TableRefiner::set_particle(kernel::Particle *p, const kernel::ParticlesTemp &ps) {
  IMP_USAGE_CHECK(map_.find(p) != map_.end(),
                  "Particle " << p->get_name() << " not found in map.");
  map_[p] = get_as<kernel::Particles>(ps);
}

bool TableRefiner::get_can_refine(kernel::Particle *p) const {
  return map_.find(p) != map_.end();
}

const kernel::ParticlesTemp TableRefiner::get_refined(kernel::Particle *p) const {
  IMP_INTERNAL_CHECK(map_.find(p) != map_.end(),
                     "Particle is not found in table to refine");
  return kernel::ParticlesTemp(map_.find(p)->second.begin(),
                       map_.find(p)->second.end());
}

ModelObjectsTemp TableRefiner::do_get_inputs(Model *m,
                                             const kernel::ParticleIndexes &pis) const {
  return IMP::kernel::get_particles(m, pis);
}

IMPCORE_END_NAMESPACE
