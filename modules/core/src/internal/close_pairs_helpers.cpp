/**
 *  \file graph_base.cpp   \brief classes for implementing a graph.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/internal/close_pairs_helpers.h>
#include <IMP/core/core_macros.h>

#include <sstream>

IMPCORE_BEGIN_INTERNAL_NAMESPACE
ParticlesTemp
get_input_particles(Model *, SingletonContainer *sc_,
                    const PairPredicates &filters_) {

  ParticlesTemp ret= sc_->get_all_possible_particles();
  {
    ParticleIndexes pis= sc_->get_all_possible_indexes();
    ParticlesTemp pisp= IMP::internal::get_particle(sc_->get_model(), pis);
    IMP_INTERNAL_CHECK(pis.size() == ret.size(), "Sizes don't mathc");
    IMP_INTERNAL_CHECK(pisp.size() == pis.size(), "Sizes don't match");
  }
  IMP_INTERNAL_CHECK(std::find(ret.begin(), ret.end(), nullptr)
                         == ret.end(), "Null all possible particle found");
  ParticlesTemp all;
  unsigned int ret_size=ret.size();
  IMP_INTERNAL_CHECK(ret_size==ret.size(), "Huh?");
  for (unsigned int i=0; i< filters_.size(); ++i) {
    for (unsigned int j=0; j< ret_size; ++j) {
      IMP_INTERNAL_CHECK(ret_size==ret.size(), "Huh?");
      IMP_INTERNAL_CHECK(j < ret.size(), "Huh? Really?");
      Particle *p= ret[j];
      ParticlesTemp cur= filters_[i]->get_input_particles(p);
      IMP_INTERNAL_CHECK(std::find(cur.begin(), cur.end(), nullptr)
                         == cur.end(), "Null input particle found");
      all.insert(all.end(), cur.begin(), cur.end());
    }
  }
  ret.insert(ret.end(), all.begin(), all.end());
  compatibility::set<Particle*> rigid;
  for (unsigned int i=0; i< ret.size(); ++i) {
    if (core::RigidMember::particle_is_instance(ret[i])) {
      Particle *rbp=core::RigidMember(ret[i]).get_rigid_body();
      if (rigid.find(rbp) == rigid.end()) {
        rigid.insert(rbp);
        ret.push_back(rbp);
      }
    }
  }
  return ret;
}

IMPCORE_END_INTERNAL_NAMESPACE
