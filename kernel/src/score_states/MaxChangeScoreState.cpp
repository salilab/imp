/**
 *  \file MaxChangeScoreState.cpp
 *  \brief Keep track of the maximum change of a set of attributes.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */


#include "IMP/score_states/MaxChangeScoreState.h"

namespace IMP
{

IMP_LIST_IMPL(MaxChangeScoreState, Particle, particle, Particle*,
              {if (0) std::cout << *obj << index;}, {reset();});

void MaxChangeScoreState::update()
{
  max_change_=0;
  for (unsigned int i=0; i < orig_.size(); ++i) {
    for (unsigned int j=0; j < keys_.size(); ++j) {
      IMP_LOG(VERBOSE, "Particle " << ps_[i]->get_index() 
              << " and attribute " << keys_[j]
              << " moved " << std::abs(ps_[i]->get_value(keys_[j])
                                       - orig_[i][j]) << std::endl);
      max_change_= std::max(max_change_,
                            std::abs(ps_[i]->get_value(keys_[j])
                                     - orig_[i][j]));
    }
  }
  IMP_LOG(TERSE, "MaxChange update got " << max_change_ << std::endl); 
}


void MaxChangeScoreState::reset()
{
  orig_.resize(ps_.size(), std::vector<float>(keys_.size(), 0));
  for (unsigned int i=0; i < orig_.size(); ++i) {
    for (unsigned int j=0; j < keys_.size(); ++j) {
      orig_[i][j]=-ps_[i]->get_value(keys_[j]);
    }
  }
  max_change_=0;
}

} // namespace IMP
