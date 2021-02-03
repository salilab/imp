/**
 *  \file AllPairContainer.cpp   \brief A list of ParticlePairs.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/AllPairContainer.h"
#include <IMP/container/AllBipartitePairContainer.h>
#include <IMP/container/PairContainerSet.h>
#include <IMP/PairModifier.h>
#include <algorithm>

IMPCONTAINER_BEGIN_NAMESPACE

AllPairContainer::AllPairContainer(SingletonContainerAdaptor c,
                                   std::string name)
    : PairContainer(c->get_model(), name), c_(c) {
  c.set_name_if_default("AllPairContainerInput0%1%");
}

ParticleIndexPairs AllPairContainer::get_indexes() const {
  ParticleIndexes ia = c_->get_indexes();
  ParticleIndexPairs ret;
  ret.reserve(ia.size() * (ia.size() - 1) / 2);
  for (unsigned int i = 0; i < ia.size(); ++i) {
    for (unsigned int j = 0; j < i; ++j) {
      ret.push_back(ParticleIndexPair(ia[i], ia[j]));
    }
  }
  return ret;
}

ParticleIndexPairs AllPairContainer::get_range_indexes() const {
  ParticleIndexes ia = c_->get_range_indexes();
  ParticleIndexPairs ret;
  ret.reserve(ia.size() * (ia.size() - 1) / 2);
  for (unsigned int i = 0; i < ia.size(); ++i) {
    for (unsigned int j = 0; j < i; ++j) {
      ret.push_back(ParticleIndexPair(ia[i], ia[j]));
    }
  }
  return ret;
}

ParticleIndexes AllPairContainer::get_all_possible_indexes() const {
  return c_->get_all_possible_indexes();
}

ModelObjectsTemp AllPairContainer::do_get_inputs() const {
  return ModelObjectsTemp(1, c_);
}

IMPCONTAINER_END_NAMESPACE
