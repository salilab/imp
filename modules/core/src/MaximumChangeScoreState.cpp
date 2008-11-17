/**
 *  \file MaximumChangeScoreState.cpp
 *  \brief Keep track of the maximumimum change of a set of attributes.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */


#include <IMP/core/MaximumChangeScoreState.h>

#include <IMP/internal/utility.h>

#include <algorithm>
#include <sstream>

IMPCORE_BEGIN_NAMESPACE

MaximumChangeScoreState::MaximumChangeScoreState(SingletonContainer *pc,
                                                 const FloatKeys &keys):
  keys_(keys), pc_(pc)
{
  reset();
}

MaximumChangeScoreState::~MaximumChangeScoreState() {}

void MaximumChangeScoreState::do_before_evaluate()
{
  IMP_CHECK_OBJECT(pc_);
  maximum_change_=0;
  for (SingletonContainer::ParticleIterator it= pc_->particles_begin();
       it != pc_->particles_end(); ++it) {
    (*it)->assert_is_valid();
    ParticleIndex pi=(*it)->get_index();
    if (orig_values_.find(pi) == orig_values_.end()) {
      maximum_change_= std::numeric_limits<Float>::max();
      break;
    } else {
      for (unsigned int j=0; j < keys_.size(); ++j) {
        Float v= (*it)->get_value(keys_[j]);
        Float ov= orig_values_[pi].get_value(keys_[j]);
        maximum_change_= std::max(maximum_change_,
                                  std::abs(v-ov));
      }
    }
  }
  IMP_LOG(TERSE, "MaximumChange update got " << maximum_change_ << std::endl);
}


void MaximumChangeScoreState::reset()
{
  maximum_change_=0;
  orig_values_.clear();
  for (SingletonContainer::ParticleIterator it= pc_->particles_begin();
       it != pc_->particles_end(); ++it) {
    (*it)->assert_is_valid();
    ParticleIndex pi=(*it)->get_index();
    orig_values_[pi]=AT();
    for (unsigned int i=0; i< keys_.size(); ++i) {
      orig_values_[pi].insert(keys_[i],
                              (*it)->get_value(keys_[i]));
    }
  }
}

void MaximumChangeScoreState::show(std::ostream &out) const
{
  out << "MaximumChangeScoreState" << std::endl;
}

IMPCORE_END_NAMESPACE
