/**
 *  \file IMP/container/DistributeTripletsScoreState.h
 *  \brief Apply a TripletScore to each Triplet in a list.
 *
 *  WARNING This file was generated from DistributeNAMEsScoreState.hpp
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCONTAINER_DISTRIBUTE_TRIPLETS_SCORE_STATE_H
#define IMPCONTAINER_DISTRIBUTE_TRIPLETS_SCORE_STATE_H

#include <IMP/container/container_config.h>

#include "DynamicListTripletContainer.h"
#include <IMP/TripletPredicate.h>
#include <IMP/TripletContainer.h>
#include <IMP/base/Vector.h>
#include <IMP/triplet_macros.h>
#include <IMP/score_state_macros.h>
#include <boost/tuple/tuple.hpp>

#include <iostream>

IMPCONTAINER_BEGIN_NAMESPACE

//! Distribute contents of one container into several based on predicates
/** This ScoreState takes a list of predicates and values. For
    each tuple in the input container, it is placed in a given output container
    if the predicate, when applied, has the passed value.

    \note The output containers contents are not necessarily disjoint.
*/
class IMPCONTAINEREXPORT DistributeTripletsScoreState :
public ScoreState
{
  base::OwnerPointer<TripletContainer> input_;
  typedef boost::tuple<base::Pointer<DynamicListTripletContainer>,
                base::OwnerPointer<TripletPredicate>, int> Data;
  base::Vector<Data> data_;
  mutable bool updated_;
  void update_lists_if_necessary() const;
public:
  DistributeTripletsScoreState(TripletContainerAdaptor input,
                      std::string name="DistributeTripletsScoreState %1%");

  /** A given tuple will go into the returned container if \c predicate
      returns \c value when applied to it.*/
  DynamicListTripletContainer *add_predicate(TripletPredicate *predicate,
                                        int value) {
    IMP_NEW(DynamicListTripletContainer, c, (input_,
                                        predicate->get_name()+ " output"));
    data_.push_back(Data(c, predicate, value));
    return c;
  }
  IMP_CONSTRAINT_2(DistributeTripletsScoreState);
};

IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_DISTRIBUTE_TRIPLETS_SCORE_STATE_H */
