/**
 *  \file IMP/container/TripletContainerStatistics.h
 *  \brief A container for Triplets.
 *
 *  WARNING This file was generated from NAMEContainerIndex.hpp
 *  in tools/maintenance/container_templates/container/internal
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_TRIPLET_CONTAINER_INDEX_H
#define IMPCONTAINER_TRIPLET_CONTAINER_INDEX_H

#include <IMP/container/container_config.h>
#include <IMP/TripletContainer.h>
#include <IMP/internal/container_helpers.h>
#include <IMP/ScoreState.h>
#include <IMP/score_state_macros.h>
#include <IMP/compatibility/set.h>

IMPCONTAINER_BEGIN_INTERNAL_NAMESPACE

/** Store an index that allows one to quickly determine of something
    is in a container.*/
class IMPCONTAINEREXPORT TripletContainerIndex : public ScoreState
{
  base::Pointer<TripletContainer> container_;
  IMP::compatibility::set<ParticleIndexTriplet> contents_;
  bool handle_permutations_;

  void build();
public:
  TripletContainerIndex(TripletContainerAdaptor c,
                          bool handle_permutations);
  bool get_contains(ParticleIndexTriplet i) const {
    if (handle_permutations_) i= IMP::kernel::internal::get_canonical(i);
    return contents_.find(i) != contents_.end();
  }
  IMP_SCORE_STATE(TripletContainerIndex);
};


IMPCONTAINER_END_INTERNAL_NAMESPACE

#endif  /* IMPCONTAINER_TRIPLET_CONTAINER_INDEX_H */
