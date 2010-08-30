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

IMPCONTAINER_BEGIN_NAMESPACE

//!
class IMPCONTAINEREXPORT ConsecutivePairContainer : public PairContainer
{
  const Particles ps_;
  template <class F>
    F foreach(F f) const {
    unsigned int szc=ps_.size();
    for (unsigned int i=1; i< szc; ++i) {
      ParticlePair p(ps_[i-1], ps_[i]);
      f(p);
    }
    return f;
  }
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
  for (unsigned int i=1; i< ps_.size(); ++i) {
    if (ps_[i]== p[1] && ps_[i-1]==p[0]) {
      return true;
    }
  }
  return false;
}

IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_CONSECUTIVE_PAIR_CONTAINER_H */
