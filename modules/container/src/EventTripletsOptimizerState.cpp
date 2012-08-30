/**
 *  \file TripletPredicate.cpp  \brief Define TripletPredicate
 *
 *  WARNING This file was generated from EventNAMEsOptimizerState.cc
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#include <IMP/container/EventTripletsOptimizerState.h>
#include <IMP/triplet_macros.h>
#include <IMP/Optimizer.h>
IMPCONTAINER_BEGIN_NAMESPACE
EventTripletsOptimizerState::
EventTripletsOptimizerState(TripletPredicate *pred,
                              TripletContainerAdaptor container,
                              int value, int min_count, int max_count,
                              std::string name):
    OptimizerState(name), pred_(pred), container_(container),
    v_(value), min_(min_count), max_(max_count) {}


void EventTripletsOptimizerState::do_show(std::ostream &out) const {
  out << "value: " << v_ << std::endl;
  out << "range: [" << min_ << "..." << max_ << ")" << std::endl;
}

void EventTripletsOptimizerState::update() {
  int met=0;
  Model *m=get_optimizer()->get_model();
  IMP_FOREACH_TRIPLET_INDEX(container_,
                               if (pred_->get_value_index(m, _1)==v_) {
                                 ++met;
                               });
  if (met >= min_ && met < max_) {
    throw IMP::base::EventException("an event occurred");
  }
}
IMPCONTAINER_END_NAMESPACE
