/**
 * \file IMP/multi_state/MultiStateModel.h
 * \brief Keep track of multiple states
 *
 * \authors Dina Schneidman
 * Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTI_STATE_MULTI_STATE_MODEL_H
#define IMPMULTI_STATE_MULTI_STATE_MODEL_H

#include <IMP/multi_state/multi_state_config.h>

#include <IMP/Vector.h>

#include <iostream>
#include <iomanip>

IMPMULTISTATE_BEGIN_NAMESPACE

//! Keep track of multiple states
class MultiStateModel {
public:
  MultiStateModel(unsigned int size) : score_(0.0), zscore_(0.0) {
    states_.reserve(size);
  }

  /* Modifiers */

  void add_state(unsigned int new_state) {
    states_.push_back(new_state);
    // invalidate score
    score_ =  0.0;
  }

  void set_score(double score) { score_ = score; }

  void set_zscore(double score) { zscore_ = score; }

  void replace_last_state(unsigned int new_state) {
    states_[states_.size()-1] = new_state;
  }

  /* Access */

  const Vector<unsigned int>& get_states() const { return states_; }

  double get_score() const { return score_; }

  double get_zscore() const { return zscore_; }

  unsigned int size() const { return states_.size(); }

  unsigned int get_last_state() const { return states_.back(); }

  friend std::ostream& operator<<(std::ostream& s, const MultiStateModel& e) {
    for(unsigned int i=0; i<e.states_.size(); i++)
      s << std::setw(4) << e.states_[i] << " ";
    s  << std::setw(6) << std::setprecision(4) << e.score_;
    return s;
  }

private:
  Vector<unsigned int> states_;
  double score_;
  double zscore_;
};

//! Utility class to help sort MultiStateModel objects
class CompareMultiStateModels {
public:
  bool operator()(const MultiStateModel& e1, const MultiStateModel& e2) const {
    return (e1.get_score() < e2.get_score());
  }
};

IMPMULTISTATE_END_NAMESPACE

#endif /* IMPMULTI_STATE_MULTI_STATE_MODEL_H */
