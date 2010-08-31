/**
 *  \file AllPairContainer.cpp   \brief A list of ParticlePairs.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/ConsecutivePairContainer.h"
#include <IMP/PairModifier.h>
#include <algorithm>


IMPCONTAINER_BEGIN_NAMESPACE
#ifdef IMP_CPC_PARTICLE_CACHE
namespace {
  // problem with multiple models
  unsigned int count=0;
}
#endif
ConsecutivePairContainer::ConsecutivePairContainer(const ParticlesTemp &ps):
  PairContainer(ps[0]->get_model(),"ConsecutivePairContainer %1%"),
  ps_(ps){
#ifdef IMP_CPC_PARTICLE_CACHE
  std::ostringstream oss;
  oss << "CPC cache " << count;
  ++count;
  key_= IntKey(oss.str());
  for (unsigned int i=0; i< ps.size(); ++i) {
    IMP_USAGE_CHECK(!ps[i]->get_has_attribute(key_),
                    "You must create containers before reading in the "
                    << "saved model");
    ps[i]->add_attribute(key_, i);
  }
#endif
}

PairContainerPair
ConsecutivePairContainer::get_added_and_removed_containers() const {
    return PairContainerPair(container::ListPairContainer
                             ::create_untracked_container(),
                             container::ListPairContainer
                             ::create_untracked_container());
}

unsigned int ConsecutivePairContainer::get_number_of_particle_pairs() const {
  return ps_.size()-1;
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

IMPCONTAINER_END_NAMESPACE
