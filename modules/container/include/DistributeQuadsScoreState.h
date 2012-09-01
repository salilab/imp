/**
 *  \file IMP/container/DistributeQuadsScoreState.h
 *  \brief Apply a QuadScore to each Quad in a list.
 *
 *  WARNING This file was generated from DistributeNAMEsScoreState.hpp
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCONTAINER_DISTRIBUTE_QUADS_SCORE_STATE_H
#define IMPCONTAINER_DISTRIBUTE_QUADS_SCORE_STATE_H

#include "container_config.h"

#include "DynamicListQuadContainer.h"
#include <IMP/QuadPredicate.h>
#include <IMP/QuadContainer.h>
#include <IMP/base/Vector.h>
#include <IMP/quad_macros.h>
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
class IMPCONTAINEREXPORT DistributeQuadsScoreState :
public ScoreState
{
  base::OwnerPointer<QuadContainer> input_;
  typedef boost::tuple<base::Pointer<DynamicListQuadContainer>,
                base::OwnerPointer<QuadPredicate>, int> Data;
  base::Vector<Data> data_;
  mutable bool updated_;
  void update_lists_if_necessary() const;
public:
  DistributeQuadsScoreState(QuadContainerAdaptor input,
                      std::string name="DistributeQuadsScoreState %1%");

  /** A given tuple will go into the returned container if \c predicate
      returns \c value when applied to it.*/
  DynamicListQuadContainer *add_predicate(QuadPredicate *predicate,
                                        int value) {
    IMP_NEW(DynamicListQuadContainer, c, (input_,
                                        predicate->get_name()+ " output"));
    data_.push_back(Data(c, predicate, value));
    return c;
  }
  IMP_SCORE_STATE(DistributeQuadsScoreState);
};

IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_DISTRIBUTE_QUADS_SCORE_STATE_H */
