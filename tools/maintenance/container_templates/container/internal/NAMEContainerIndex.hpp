/**
 *  \file IMP/container/CLASSNAMEContainerStatistics.h
 *  \brief A container for CLASSNAMEs.
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_HEADERNAME_CONTAINER_INDEX_H
#define IMPCONTAINER_HEADERNAME_CONTAINER_INDEX_H

#include <IMP/container/container_config.h>
#include <IMP/CLASSNAMEContainer.h>
#include <IMP/internal/container_helpers.h>
#include <IMP/ScoreState.h>
#include <IMP/score_state_macros.h>
#include <IMP/compatibility/set.h>

IMPCONTAINER_BEGIN_INTERNAL_NAMESPACE

/** Store an index that allows one to quickly determine of something
    is in a container.*/
class IMPCONTAINEREXPORT CLASSNAMEContainerIndex : public ScoreState
{
  base::Pointer<CLASSNAMEContainer> container_;
  IMP::compatibility::set<INDEXTYPE> contents_;
  bool handle_permutations_;

  void build();
public:
  CLASSNAMEContainerIndex(CLASSNAMEContainerAdaptor c,
                          bool handle_permutations);
  bool get_contains(INDEXTYPE i) const {
    if (handle_permutations_) i= IMP::kernel::internal::get_canonical(i);
    return contents_.find(i) != contents_.end();
  }
  IMP_SCORE_STATE(CLASSNAMEContainerIndex);
};


IMPCONTAINER_END_INTERNAL_NAMESPACE

#endif  /* IMPCONTAINER_HEADERNAME_CONTAINER_INDEX_H */
