/**
 *  \file AllPairContainer.cpp   \brief A list of ParticlePairs.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
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
                                                   bool no_overlaps,
                                                   std::string name):
    PairContainer(ps[0]->get_model(),name),
    ps_(IMP::internal::get_index(ps)) {
  init(no_overlaps);
}
ConsecutivePairContainer::ConsecutivePairContainer(const ParticlesTemp &ps,
                                                   std::string name):
    PairContainer(ps[0]->get_model(),name),
    ps_(IMP::internal::get_index(ps)) {
  init(false);
}

void ConsecutivePairContainer::init(bool no_overlaps){
  if (!no_overlaps) {
    std::ostringstream oss;
    oss << "CPC cache " << count;
    ++count;
    key_= IntKey(oss.str());
  } else {
    key_= IntKey("CPC cache");
  }
  for (unsigned int i=0; i< ps_.size(); ++i) {
    IMP_USAGE_CHECK(!get_model()->get_has_attribute(key_, ps_[i]),
                    "You must create containers before reading in the "
                    << "saved model: "
                    << get_model()->get_particle(ps_[i])->get_name());
    get_model()->add_attribute(key_, ps_[i], i);
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

bool
ConsecutivePairContainer
::get_contains_particle_pair(const ParticlePair &p) const {
  if (!p[0]->has_attribute(key_)) return false;
  int ia= p[0]->get_value(key_);
  if (!p[1]->has_attribute(key_)) return false;
  int ib= p[1]->get_value(key_);
  return std::abs(ia-ib)==1;
}

ExclusiveConsecutivePairContainer
::ExclusiveConsecutivePairContainer(const ParticlesTemp &ps,
                                    std::string name):
    ConsecutivePairContainer(ps, true, name){}


ParticlesTemp ExclusiveConsecutivePairFilter
::get_input_particles(Particle*p) const {
  return ParticlesTemp(1,p);
}
ContainersTemp ExclusiveConsecutivePairFilter
::get_input_containers(Particle*) const {
  return ContainersTemp();
}

void ExclusiveConsecutivePairFilter
::do_show(std::ostream &) const {
}
IMPCONTAINER_END_NAMESPACE
