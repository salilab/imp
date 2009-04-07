/**
 *  \file TableRefiner.cpp
 *  \brief A particle refiner that uses a table.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/core/TableRefiner.h"


IMPCORE_BEGIN_NAMESPACE

TableRefiner::TableRefiner( ){
}

TableRefiner::~TableRefiner(){}

void TableRefiner::show(std::ostream &out) const {
  out << "TableRefiner" << std::endl;
}

void TableRefiner::add_particle(Particle *p,
                                        const Particles &ps) {
  IMP_check(map_.find(p) == map_.end(),
            "Particle " << p->get_name() << " already in map.",
            ValueException);
  map_[p]=ps;
}

void TableRefiner::remove_particle(Particle *p) {
  IMP_check(map_.find(p) != map_.end(),
            "Particle " << p->get_name() << " not found in map.",
            ValueException);
  map_.erase(p);
}

void TableRefiner::set_particle(Particle *p, const Particles &ps) {
  IMP_check(map_.find(p) != map_.end(),
            "Particle " << p->get_name() << " not found in map.",
            ValueException);
  map_[p]=ps;
}

bool TableRefiner::get_can_refine(Particle *p) const {
  return map_.find(p) != map_.end();
}

Particle* TableRefiner::get_refined(Particle *p, unsigned int i) const {
  IMP_assert(map_.find(p) != map_.end(),
             "Particle " << p->get_name() << " not found in map.");
  return map_.find(p)->second[i];
}


unsigned int TableRefiner::get_number_of_refined(Particle *p) const {
  IMP_assert(map_.find(p) != map_.end(),
             "Particle " << p->get_name() << " not found in map.");
  return map_.find(p)->second.size();
}

IMPCORE_END_NAMESPACE
