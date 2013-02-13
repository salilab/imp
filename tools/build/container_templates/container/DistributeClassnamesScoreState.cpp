/**
 *  \file PredicateClassnamesRestraint.cpp   \brief Container for classname.
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/DistributeClassnamesScoreState.h"

IMPCONTAINER_BEGIN_NAMESPACE
DistributeClassnamesScoreState::
DistributeClassnamesScoreState(ClassnameContainerAdaptor input,
                               std::string name): ScoreState(name) {
  input_=input;
  updated_=false;
}

ModelObjectsTemp DistributeClassnamesScoreState
::do_get_outputs() const {
  ModelObjectsTemp ret;
  for (unsigned int i=0; i< data_.size(); ++i) {
    ret.push_back(data_[i].get<0>());
  }
  return ret;
}

ModelObjectsTemp DistributeClassnamesScoreState
::do_get_inputs() const {
  ModelObjectsTemp ret;
  ParticleIndexes pis= input_->get_all_possible_indexes();
  for (unsigned int i=0; i< data_.size(); ++i) {
    ret+=data_[i].get<1>()->get_inputs(get_model(), pis);
  }
  ret.push_back(input_);
  return ret;
}



void DistributeClassnamesScoreState::do_before_evaluate() {
  update_lists_if_necessary();
}
void DistributeClassnamesScoreState
::do_after_evaluate(DerivativeAccumulator *) {
}

void DistributeClassnamesScoreState
::update_lists_if_necessary() const {
  if (updated_ && !input_->get_is_changed()) return;
  updated_=true;

  base::Vector<PLURALINDEXTYPE> output(data_.size());
  IMP_FOREACH_CLASSNAME_INDEX(input_, {
      for (unsigned int i=0; i< data_.size(); ++i) {
        if (data_[i].get<1>()->get_value_index(get_model(), _1)
            == data_[i].get<2>()) {
          output[i].push_back(_1);
        }
      }
    });
  for (unsigned int i=0; i< output.size(); ++i) {
    data_[i].get<0>()->set(output[i]);
  }
}

void DistributeClassnamesScoreState::do_show(std::ostream &) const {
}
IMPCONTAINER_END_NAMESPACE
