/**
 *  \file AllPairContainer.cpp   \brief A list of ParticlePairs.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/ConsecutivePairContainer.h"
#include <IMP/PairModifier.h>
#include <IMP/internal/container_helpers.h>
#include <algorithm>


IMPCONTAINER_BEGIN_NAMESPACE
namespace {
  // problem with multiple models
  unsigned int count=0;
}
ConsecutivePairContainer::ConsecutivePairContainer(const ParticlesTemp &ps,
                                                   bool no_overlaps):
  PairContainer(ps[0]->get_model(),"ConsecutivePairContainer %1%"),
  ps_(IMP::internal::get_index(ps)){
  if (!no_overlaps) {
    std::ostringstream oss;
    oss << "CPC cache " << count;
    ++count;
    key_= IntKey(oss.str());
  } else {
    key_= IntKey("CPC cache");
  }
  for (unsigned int i=0; i< ps.size(); ++i) {
    IMP_USAGE_CHECK(!ps[i]->has_attribute(key_),
                    "You must create containers before reading in the "
                    << "saved model");
    ps[i]->add_attribute(key_, i);
  }
}

bool
ConsecutivePairContainer::get_contents_changed() const {
  return false;
}

ParticleIndexPairs ConsecutivePairContainer::get_indexes() const {
  ParticleIndexPairs ret(ps_.size()-1);
  for (unsigned int i=1; i< ps_.size(); ++i) {
    ret[i-1]= ParticleIndexPair(ps_[i-1], ps_[i]);
  }
  return ret;
}

ParticleIndexPairs ConsecutivePairContainer::get_all_possible_indexes() const {
  return get_indexes();
}


void ConsecutivePairContainer::do_show(std::ostream &out) const {
  IMP_CHECK_OBJECT(this);
  out << "num particles: " << ps_.size() << std::endl;
}

ParticlesTemp ConsecutivePairContainer::get_contained_particles() const {
  return IMP::internal::get_particle(get_model(), ps_);
}

IMPCONTAINER_END_NAMESPACE
