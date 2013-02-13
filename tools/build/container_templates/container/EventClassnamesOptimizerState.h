/**
 *  \file IMP/container/EventClassnamesOptimizerState.h
 *  \brief Define some predicates.
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_EVENT_CLASSNAMES_OPTIMIZER_STATE_H
#define IMPCONTAINER_EVENT_CLASSNAMES_OPTIMIZER_STATE_H

#include <IMP/container/container_config.h>
#include <IMP/ClassnamePredicate.h>
#include <IMP/OptimizerState.h>
#include <IMP/ClassnameContainer.h>
#include <IMP/optimizer_state_macros.h>

IMPCONTAINER_BEGIN_NAMESPACE

/** Raise an IMP::base::EventException when a certain condition is met.
    Currently the supported logic is when the number of items in the
    container for which the predicate returns a certain value is in the
    range [min_count, max_count).
 */
class IMPCONTAINEREXPORT EventClassnamesOptimizerState:
    public OptimizerState {
  IMP::OwnerPointer<ClassnamePredicate> pred_;
  IMP::OwnerPointer<ClassnameContainer> container_;
  int v_;
  int min_, max_;
public:
  EventClassnamesOptimizerState(ClassnamePredicate *pred,
                                ClassnameContainerAdaptor container,
                                int value, int min_count, int max_count,
                                std::string name="ConstClassnamePredicate%1%");
  IMP_OPTIMIZER_STATE(EventClassnamesOptimizerState);
};

IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_EVENT_CLASSNAMES_OPTIMIZER_STATE_H */
