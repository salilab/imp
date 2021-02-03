/**
 *  \file AllPairContainer.cpp   \brief A list of ParticlePairs.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/ConsecutivePairContainer.h"
#include <IMP/PairModifier.h>
#include <IMP/internal/container_helpers.h>
#include <algorithm>

IMPCONTAINER_BEGIN_NAMESPACE
namespace {
// global counter to differentiate keys of overlapping containers
// TODO: why not a static class variable?
unsigned int key_count = 0;
}

ConsecutivePairContainer::ConsecutivePairContainer(
    Model *m, const ParticleIndexes &ps, std::string name)
    : PairContainer(m, name), ps_(ps) {
  init();
}

// add key of this container as attribute to all particles
// if there might be overlaps - create a different keys for each instance
void ConsecutivePairContainer::init() {
  std::ostringstream oss;
  oss << "CPC cache " << key_count;
  ++key_count;
  key_ = IntKey(oss.str());
  for (unsigned int i = 0; i < ps_.size(); ++i) {
    IMP_USAGE_CHECK(!get_model()->get_has_attribute(key_, ps_[i]),
                    "You must create containers before reading in the "
                        << "saved model: "
                        << get_model()->get_particle(ps_[i])->get_name());
    get_model()->add_attribute(key_, ps_[i], i);
  }
}

ModelObjectsTemp ConsecutivePairContainer::do_get_inputs() const {
  return ParticlesTemp();
}

ParticleIndexPairs ConsecutivePairContainer::get_indexes() const {
  ParticleIndexPairs ret(ps_.size() - 1);
  for (unsigned int i = 1; i < ps_.size(); ++i) {
    ret[i - 1] = ParticleIndexPair(ps_[i - 1], ps_[i]);
  }
  return ret;
}

ParticleIndexPairs ConsecutivePairContainer::get_range_indexes() const {
  return get_indexes();
}

ParticleIndexes ConsecutivePairContainer::get_all_possible_indexes() const {
  return ps_;
}

ConsecutivePairFilter::ConsecutivePairFilter(ConsecutivePairContainer *cpc)
    : PairPredicate("ConsecutivePairFilter %1%"), cpc_(cpc) {}

ExclusiveConsecutivePairContainer::ExclusiveConsecutivePairContainer(
    Model *m, const ParticleIndexes &ps, std::string name)
    : PairContainer(m, name), ps_(ps) {
  init();
}

// add key of this container as attribute to all particles
// if there might be overlaps - create a different keys for each instance
void ExclusiveConsecutivePairContainer::init() {
  for (unsigned int i = 0; i < ps_.size(); ++i) {
    IMP_USAGE_CHECK(
        !get_model()->get_has_attribute(get_exclusive_key(), ps_[i]),
        "Particle already added to some ExclusiveConsecutivePairContainer"
        << " and cannot be added to another: "
            << get_model()->get_particle(ps_[i])->get_name() );
    get_model()->add_attribute(get_exclusive_key(), ps_[i], i);
    get_model()->add_attribute(get_exclusive_object_key(), ps_[i], this);
  }
}

void ExclusiveConsecutivePairContainer::do_destroy() {
  if(get_model() == nullptr){ return; }
  if(!get_model()->get_is_valid()) { return ; }
  IMP_LOG_PROGRESS("Destroying exclusive pair container with "
                   << ps_.size() << " particles" << std::endl);
  for (unsigned int i = 0; i < ps_.size(); ++i) {
    if(!get_model()->get_has_particle(ps_[i])) {
        continue;
    }
    if( get_model()->get_has_attribute(get_exclusive_key(), ps_[i]) ) {
        get_model()->remove_attribute(get_exclusive_key(), ps_[i]);
    }
    if( get_model()->get_has_attribute(get_exclusive_object_key(), ps_[i]) ){
      get_model()->remove_attribute(get_exclusive_object_key(), ps_[i]);
    }
  }
}

ModelObjectsTemp ExclusiveConsecutivePairContainer::do_get_inputs() const {
  return ParticlesTemp();
}

ParticleIndexPairs ExclusiveConsecutivePairContainer::get_indexes() const {
  ParticleIndexPairs ret(ps_.size() - 1);
  for (unsigned int i = 1; i < ps_.size(); ++i) {
    ret[i - 1] = ParticleIndexPair(ps_[i - 1], ps_[i]);
  }
  return ret;
}

ParticleIndexPairs ExclusiveConsecutivePairContainer::get_range_indexes()
    const {
  return get_indexes();
}

ParticleIndexes ExclusiveConsecutivePairContainer::get_all_possible_indexes()
    const {
  return ps_;
}

IMPCONTAINER_END_NAMESPACE
