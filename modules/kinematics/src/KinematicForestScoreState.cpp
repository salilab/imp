/**
 * \file KinematicForestScoreState.cpp
 * \brief
 *
 * \authors Dina Schneidman, Barak Raveh
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/kinematics/KinematicForestScoreState.h>

IMPKINEMATICS_BEGIN_NAMESPACE

void KinematicForestScoreState::do_show(std::ostream &) const {}

ModelObjectsTemp KinematicForestScoreState::do_get_inputs() const {
  ModelObjectsTemp ret;
  ret += atoms_;
  for(unsigned int i=0; i<rbs_.size(); i++) {
    ret.push_back(rbs_[i]);
  }
  return ret;
}

ModelObjectsTemp KinematicForestScoreState::do_get_outputs() const {
  ModelObjectsTemp ret;
  ret+= atoms_;
  for(unsigned int i=0; i<rbs_.size(); i++) {
    ret.push_back(rbs_[i]);
  }
  return ret;
}



IMPKINEMATICS_END_NAMESPACE
