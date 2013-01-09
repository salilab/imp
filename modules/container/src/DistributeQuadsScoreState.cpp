/**
 *  \file PredicateQuadsRestraint.cpp   \brief Container for quad.
 *
 *  WARNING This file was generated from DistributeNAMEsScoreState.cc
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/DistributeQuadsScoreState.h"

IMPCONTAINER_BEGIN_NAMESPACE
DistributeQuadsScoreState::
DistributeQuadsScoreState(QuadContainerAdaptor input,
                               std::string name): ScoreState(name) {
  input_=input;
  updated_=false;
}

ModelObjectsTemp DistributeQuadsScoreState
::do_get_outputs() const {
  ModelObjectsTemp ret;
  for (unsigned int i=0; i< data_.size(); ++i) {
    ret.push_back(data_[i].get<0>());
  }
  return ret;
}

ModelObjectsTemp DistributeQuadsScoreState
::do_get_inputs() const {
  ModelObjectsTemp ret;
  ParticleIndexes pis= input_->get_all_possible_indexes();
  for (unsigned int i=0; i< data_.size(); ++i) {
    ret+=data_[i].get<1>()->get_inputs(get_model(), pis);
  }
  ret.push_back(input_);
  return ret;
}



void DistributeQuadsScoreState::do_before_evaluate() {
  update_lists_if_necessary();
}
void DistributeQuadsScoreState
::do_after_evaluate(DerivativeAccumulator *) {
}

void DistributeQuadsScoreState
::update_lists_if_necessary() const {
  if (updated_ && !input_->get_is_changed()) return;
  updated_=true;

  base::Vector<ParticleIndexQuads> output(data_.size());
  IMP_FOREACH_QUAD_INDEX(input_, {
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

void DistributeQuadsScoreState::do_show(std::ostream &) const {
}
IMPCONTAINER_END_NAMESPACE
