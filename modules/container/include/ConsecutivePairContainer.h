/**
 *  \file ConsecutivePairContainer.h
 *  \brief Return all pairs from a SingletonContainer
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_CONSECUTIVE_PAIR_CONTAINER_H
#define IMPCONTAINER_CONSECUTIVE_PAIR_CONTAINER_H

#include "container_config.h"

#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/container/ListPairContainer.h>
#include <IMP/compatibility/map.h>

IMPCONTAINER_BEGIN_NAMESPACE

//! A container which contains all consecutive pairs from a list
class IMPCONTAINEREXPORT ConsecutivePairContainer : public PairContainer
{
  const ParticleIndexes ps_;
#define IMP_CP_LOOP(body)                       \
  for (unsigned int i=1; i< ps_.size(); ++i) {  \
    ParticleIndexPair item(ps_[i-1], ps_[i]);   \
    body;                                       \
  }
 IMP_IMPLEMENT_PAIR_CONTAINER_OPERATIONS(ConsecutivePairContainer,
                                               IMP_CP_LOOP);

#undef IMP_CP_LOOP

  IntKey key_;
public:
  //! Get the individual particles from the passed SingletonContainer
  /** If no overlaps is true, this CPC is assumed not to share any particles
      with another CPC.
  */
  ConsecutivePairContainer(const ParticlesTemp &ps, bool no_overlaps=false);

#ifndef IMP_DOXYGEN
  bool get_is_up_to_date() const {
    return true;
  }
#endif
  IMP_PAIR_CONTAINER(ConsecutivePairContainer);
};

IMP_OBJECTS(ConsecutivePairContainer,ConsecutivePairContainers);


inline bool
ConsecutivePairContainer
::get_contains_particle_pair(const ParticlePair &p) const {
  if (!p[0]->has_attribute(key_)) return false;
  int ia= p[0]->get_value(key_);
  if (!p[1]->has_attribute(key_)) return false;
  int ib= p[1]->get_value(key_);
  return std::abs(ia-ib)==1;

}

IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_CONSECUTIVE_PAIR_CONTAINER_H */
