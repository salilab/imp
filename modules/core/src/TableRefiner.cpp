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

void TableRefiner::do_show(std::ostream &out) const {
  out << "\n  " << map_.size() << " entries";
}

void TableRefiner::add_particle(Particle *p, const ParticlesTemp &ps) {
  IMP_USAGE_CHECK(map_.find(p) == map_.end(),
                  "Particle " << p->get_name() << " already in map.");
  map_[p] = get_as<Particles>(ps);
}

void TableRefiner::remove_particle(Particle *p) {
  IMP_USAGE_CHECK(map_.find(p) != map_.end(),
                  "Particle " << p->get_name() << " not found in map.");
  map_.erase(p);
}

void TableRefiner::set_particle(Particle *p, const ParticlesTemp &ps) {
  IMP_USAGE_CHECK(map_.find(p) != map_.end(),
                  "Particle " << p->get_name() << " not found in map.");
  map_[p] = get_as<Particles>(ps);
}

bool TableRefiner::get_can_refine(Particle *p) const {
  return map_.find(p) != map_.end();
}

Particle *TableRefiner::get_refined(Particle *p, unsigned int i) const {
  IMP_INTERNAL_CHECK(map_.find(p) != map_.end(),
                     "Particle " << p->get_name() << " not found in map.");
  return map_.find(p)->second[i];
}

unsigned int TableRefiner::get_number_of_refined(Particle *p) const {
  IMP_INTERNAL_CHECK(map_.find(p) != map_.end(),
                     "Particle " << p->get_name() << " not found in map.");
  return map_.find(p)->second.size();
}

const ParticlesTemp TableRefiner::get_refined(Particle *p) const {
  IMP_INTERNAL_CHECK(map_.find(p) != map_.end(),
                     "Particle is not found in table to refine");
  return ParticlesTemp(map_.find(p)->second.begin(),
                       map_.find(p)->second.end());
}

ParticlesTemp TableRefiner::get_input_particles(Particle *p) const {
  ParticlesTemp ret = get_refined(p);
  ret.push_back(p);
  return ret;
}

ContainersTemp TableRefiner::get_input_containers(Particle *) const {
  return ContainersTemp();
}

IMPCORE_END_NAMESPACE
