/**
 *  \file ConsecutivePairContainer.h
 *  \brief Return all pairs from a SingletonContainer
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_CONSECUTIVE_PAIR_CONTAINER_H
#define IMPCONTAINER_CONSECUTIVE_PAIR_CONTAINER_H

#include "container_config.h"
#include <IMP/generic.h>
#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/container/ListPairContainer.h>
#include <IMP/compatibility/map.h>

IMPCONTAINER_BEGIN_NAMESPACE

//! A container which contains all consecutive pairs from a list
/** If each of your particles will only be in one such container,
    you may want to use the ExclusiveConsecutivePairContainer instead.
*/
class IMPCONTAINEREXPORT ConsecutivePairContainer : public PairContainer
{
  friend class ExclusiveConsecutivePairFilter;
  friend class ExclusiveConsecutivePairContainer;
  static IntKey get_exclusive_key() {
    static IntKey k("exclusive consecutive numbering");
    return k;
  }
  const ParticleIndexes ps_;
  IntKey key_;
  void init(bool no_overlaps);
  ConsecutivePairContainer(const ParticlesTemp &ps,
                           bool no_overlaps,
                           std::string name="ConsecutivePairContainer%1%");
#define IMP_CP_LOOP(body)                       \
  for (unsigned int i=1; i< ps_.size(); ++i) {  \
    ParticleIndexPair item(ps_[i-1], ps_[i]);   \
    body;                                       \
  }
 IMP_IMPLEMENT_PAIR_CONTAINER_OPERATIONS(ConsecutivePairContainer,
                                               IMP_CP_LOOP);

#undef IMP_CP_LOOP
public:
  //! Get the individual particles from the passed SingletonContainer
  ConsecutivePairContainer(const ParticlesTemp &ps,
                           std::string name="ConsecutivePairContainer%1%");

#ifndef IMP_DOXYGEN
  bool get_is_up_to_date() const {
    return true;
  }
#endif
  IMP_PAIR_CONTAINER(ConsecutivePairContainer);
};

IMP_OBJECTS(ConsecutivePairContainer,ConsecutivePairContainers);

/** This is an ConsecutivePairContainer where each particle can only be on
    one ExclusiveConsecutivePairContainer. The exclusivity makes the code
    more efficient and allows one to use the ExclusiveConsecutivePairFilter,
    which is way more efficient than using an InContainerPairFilter
    with a ConsecutivePairContainer.*/
class IMPCONTAINEREXPORT ExclusiveConsecutivePairContainer :
public ConsecutivePairContainer
{
public:
  //! Get the individual particles from the passed SingletonContainer
  ExclusiveConsecutivePairContainer(const ParticlesTemp &ps,
          std::string name="ExclusiveConsecutivePairContainer%1%");

};

/** Check for whether the pair is a member of any
    ExclusiveConsecutivePairContainer. */
class IMPCONTAINEREXPORT ExclusiveConsecutivePairFilter: public PairFilter {
 public:
  ExclusiveConsecutivePairFilter():
      PairFilter("ExclusiveConsecutivePairFilter %1%"){}
  IMP_INDEX_PAIR_FILTER(ExclusiveConsecutivePairFilter);
};

inline bool ExclusiveConsecutivePairFilter
::get_contains(Model *m, const ParticleIndexPair &pp) const {
  IntKey k= ConsecutivePairContainer::get_exclusive_key();
  if (!m->get_has_attribute(k, pp[0])) return false;
  int ia= m->get_attribute(k, pp[0]);
  if (!m->get_has_attribute(k, pp[1])) return false;
  int ib= m->get_attribute(k, pp[1]);
  return std::abs(ia-ib)==1;
}




IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_CONSECUTIVE_PAIR_CONTAINER_H */
