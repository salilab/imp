/**
 *  \file IMP/container/SingletonContainerStatistics.h
 *  \brief A container for Singletons.
 *
 *  WARNING This file was generated from NAMEContainerIndex.hpp
 *  in tools/maintenance/container_templates/container/internal
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_SINGLETON_CONTAINER_INDEX_H
#define IMPCONTAINER_SINGLETON_CONTAINER_INDEX_H

#include <IMP/container/container_config.h>
#include <IMP/SingletonContainer.h>
#include <IMP/internal/container_helpers.h>
#include <IMP/ScoreState.h>
#include <IMP/score_state_macros.h>
#include <IMP/compatibility/set.h>

IMPCONTAINER_BEGIN_INTERNAL_NAMESPACE

/** Store an index that allows one to quickly determine of something
    is in a container.*/
class IMPCONTAINEREXPORT SingletonContainerIndex : public ScoreState
{
  base::Pointer<SingletonContainer> container_;
  IMP::compatibility::set<ParticleIndex> contents_;
  bool handle_permutations_;

  void build();
public:
  SingletonContainerIndex(SingletonContainerAdaptor c,
                          bool handle_permutations);
  bool get_contains(ParticleIndex i) const {
    if (handle_permutations_) i= IMP::kernel::internal::get_canonical(i);
    return contents_.find(i) != contents_.end();
  }
  IMP_SCORE_STATE(SingletonContainerIndex);
};


IMPCONTAINER_END_INTERNAL_NAMESPACE

#endif  /* IMPCONTAINER_SINGLETON_CONTAINER_INDEX_H */
