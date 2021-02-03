/**
 *  \file close_pairs_helpers.cpp   \brief Helper functions for close pairs.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/internal/close_pairs_helpers.h>

#include <sstream>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

IntKey core::internal::InList::key_ = IntKey("in list temp");

ModelObjectsTemp get_inputs(Model *m, SingletonContainer *sc_,
                            const PairPredicates &filters_) {
  ParticleIndexes all = sc_->get_all_possible_indexes();
  ModelObjectsTemp ret = IMP::get_particles(m, all);
  for (unsigned int i = 0; i < filters_.size(); ++i) {
    ret += filters_[i]->get_inputs(m, all);
  }
  boost::unordered_set<Particle *> rigid;
  for (unsigned int i = 0; i < all.size(); ++i) {
    if (core::RigidMember::get_is_setup(m, all[i])) {
      Particle *rbp = core::RigidMember(m, all[i]).get_rigid_body();
      if (rigid.find(rbp) == rigid.end()) {
        rigid.insert(rbp);
        ret.push_back(rbp);
      }
    }
  }
  ret.push_back(sc_);
  return ret;
}

IMPCORE_END_INTERNAL_NAMESPACE
