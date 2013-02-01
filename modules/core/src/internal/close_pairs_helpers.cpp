/**
 *  \file graph_base.cpp   \brief classes for implementing a graph.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/internal/close_pairs_helpers.h>
#include <IMP/core/core_macros.h>

#include <sstream>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

ModelObjectsTemp
get_inputs(Model *m, SingletonContainer *sc_,
                    const PairPredicates &filters_) {
  ParticleIndexes all=sc_->get_all_possible_indexes();
  ModelObjectsTemp ret= IMP::get_particles(m, all);
  for (unsigned int i=0; i< filters_.size(); ++i) {
    ret+= filters_[i]->get_inputs(m, all);
  }
  base::set<Particle*> rigid;
  for (unsigned int i=0; i< all.size(); ++i) {
    if (core::RigidMember::particle_is_instance(m, all[i])) {
      Particle *rbp=core::RigidMember(m, all[i]).get_rigid_body();
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
