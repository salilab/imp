/**
 *  \file AllPairContainer.cpp   \brief A list of ParticlePairs.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/ConsecutivePairContainer.h"
#include <IMP/PairModifier.h>
#include <algorithm>

#define FOREACH(expr)                                                   \
  unsigned int szc=ps_.size();                                           \
  for (unsigned int i=1; i< szc; ++i) {                                 \
    ParticlePair p(ps_[i-1], ps_[i]);                                    \
    expr;                                                               \
  }


IMPCONTAINER_BEGIN_NAMESPACE
ConsecutivePairContainer::ConsecutivePairContainer(const ParticlesTemp &ps):
  PairContainer(ps[0]->get_model(),"ConsecutivePairContainer %1%"),
  ps_(ps){
  set_added_and_removed_containers(
     new container::ListPairContainer(ps[0]->get_model()),
     new container::ListPairContainer(ps[0]->get_model()));
}

unsigned int ConsecutivePairContainer::get_number_of_particle_pairs() const {
  return ps_.size()-1;
}


ParticlePair ConsecutivePairContainer::get_particle_pair(unsigned int i) const {
  unsigned int ip1= i+1;
  Particle *p0= ps_[i];
  Particle *p1= ps_[ip1];
  return ParticlePair(p0, p1);
}

bool
ConsecutivePairContainer
::get_contains_particle_pair(const ParticlePair &p) const {
  for (unsigned int i=1; i< ps_.size(); ++i) {
    if (ps_[i]== p[1] && ps_[i-1]==p[0]) {
      return true;
    }
  }
  return false;
}

void ConsecutivePairContainer::do_show(std::ostream &out) const {
  IMP_CHECK_OBJECT(this);
}

ParticlesTemp ConsecutivePairContainer::get_contained_particles() const {
  return ps_;
}

bool ConsecutivePairContainer::get_contained_particles_changed() const {
  return false;
}

IMP_PAIR_CONTAINER_METHODS_FROM_FOREACH(ConsecutivePairContainer);


IMPCONTAINER_END_NAMESPACE
