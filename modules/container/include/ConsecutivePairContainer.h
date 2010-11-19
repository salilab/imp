/**
 *  \file ConsecutivePairContainer.h
 *  \brief Return all pairs from a SingletonContainer
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_CONSECUTIVE_PAIR_CONTAINER_H
#define IMPCONTAINER_CONSECUTIVE_PAIR_CONTAINER_H

#include "container_config.h"

#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/container/ListPairContainer.h>
#include <IMP/internal/map.h>

IMPCONTAINER_BEGIN_NAMESPACE
#define IMP_CPC_METHOD 0
//! A container which contains all consecutive pairs from a list
class IMPCONTAINEREXPORT ConsecutivePairContainer : public PairContainer
{
  const Particles ps_;
  template <class F>
    void apply_to_contents(F f) const {
    unsigned int szc=ps_.size();
    for (unsigned int i=1; i< szc; ++i) {
      ParticlePair p(ps_[i-1], ps_[i]);
      f(p);
    }
  }
  template <class F>
    double accumulate_over_contents(F f) const {
    double ret=0;
    unsigned int szc=ps_.size();
    for (unsigned int i=1; i< szc; ++i) {
      ParticlePair p(ps_[i-1], ps_[i]);
      ret+=f(p);
    }
    return ret;
  }
#if IMP_CPC_METHOD ==0
  IntKey key_;
#elif IMP_CPC_METHOD ==1
  IMP::internal::Map<Particle*, int> index_;
#else

#endif
public:
  //! Get the individual particles from the passed SingletonContainer
  ConsecutivePairContainer(const ParticlesTemp &ps);

#ifndef IMP_DOXYGEN
  bool get_is_up_to_date() const {
    return true;
  }
#endif
  IMP_PAIR_CONTAINER(ConsecutivePairContainer);
};

IMP_OBJECTS(ConsecutivePairContainer,ConsecutivePairContainers);


inline ParticlePair
ConsecutivePairContainer::get_particle_pair(unsigned int i) const {
  unsigned int ip1= i+1;
  Particle *p0= ps_[i];
  Particle *p1= ps_[ip1];
  return ParticlePair(p0, p1);
}

inline bool
ConsecutivePairContainer
::get_contains_particle_pair(const ParticlePair &p) const {
#if IMP_CPC_METHOD==0
  if (!p[0]->has_attribute(key_)) return false;
  int ia= p[0]->get_value(key_);
  if (!p[1]->has_attribute(key_)) return false;
  int ib= p[1]->get_value(key_);
  return std::abs(ia-ib)==1;
#elif IMP_CPC_METHOD==1
  IMP::internal::Map<Particle*, int>::const_iterator it0= index_.find(p[0]);
  if (it0==index_.end()) return false;
  IMP::internal::Map<Particle*, int>::const_iterator it1= index_.find(p[1]);
  if (it1==index_.end()) return false;
  else return std::abs(it0->second-it1->second) ==1;
#else
  for (unsigned int i=1; i< ps_.size(); ++i) {
    if (ps_[i]== p[1] && ps_[i-1]==p[0]) {
      return true;
    }
  }
  return false;
#endif
}

IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_CONSECUTIVE_PAIR_CONTAINER_H */
