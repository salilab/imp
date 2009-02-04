/**
 *  \file TableParticleRefiner.cpp
 *  \brief A particle refiner that uses a table.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/core/TableParticleRefiner.h"


IMPCORE_BEGIN_NAMESPACE

TableParticleRefiner::TableParticleRefiner( ){
}

TableParticleRefiner::~TableParticleRefiner(){}

void TableParticleRefiner::show(std::ostream &out) const {
  out << "TableParticleRefiner" << std::endl;
}

void TableParticleRefiner::add_particle(Particle *p,
                                        const Particles &ps) {
  IMP_check(map_.find(p) == map_.end(),
            "Particle " << p->get_name() << " already in map.",
            ValueException);
  map_[p]=ps;
}

void TableParticleRefiner::remove_particle(Particle *p) {
  IMP_check(map_.find(p) != map_.end(),
            "Particle " << p->get_name() << " not found in map.",
            ValueException);
  map_.erase(p);
}

void TableParticleRefiner::set_particle(Particle *p, const Particles &ps) {
  IMP_check(map_.find(p) != map_.end(),
            "Particle " << p->get_name() << " not found in map.",
            ValueException);
  map_[p]=ps;
}

bool TableParticleRefiner::get_can_refine(Particle *p) const {
  return map_.find(p) != map_.end();
}

Particles TableParticleRefiner::get_refined(Particle *p) const {
  IMP_assert(map_.find(p) != map_.end(),
             "Particle " << p->get_name() << " not found in map.");
  return map_.find(p)->second;
}

void TableParticleRefiner::cleanup_refined(Particle *p,
                                           Particles &,
                                           DerivativeAccumulator*) const {
}

IMPCORE_END_NAMESPACE
