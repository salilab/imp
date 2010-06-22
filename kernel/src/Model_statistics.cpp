/**
 *  \file Model.cpp \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/Model.h"
#include "IMP/Particle.h"
#include "IMP/log.h"
#include "IMP/Restraint.h"
#include "IMP/DerivativeAccumulator.h"
#include "IMP/ScoreState.h"
#include "IMP/RestraintSet.h"
#include <boost/timer.hpp>
#include <set>



IMP_BEGIN_NAMESPACE


namespace {
  struct Statistics {
    double total_time_;
    double total_time_after_;
    unsigned int calls_;
    double total_value_;
    double min_value_;
    double max_value_;
    Statistics(): total_time_(0), total_time_after_(0),
                  calls_(0), total_value_(0),
                  min_value_(std::numeric_limits<double>::max()),
                  max_value_(-std::numeric_limits<double>::max())
    {}
    void update_state_before(double t) {
      total_time_+=t;
    }
    void update_state_after(double t) {
      total_time_after_+=t;
      ++calls_;
    }
    void update_restraint(double t, double v) {
      total_time_+= t;
      min_value_= std::min(v, min_value_);
      max_value_= std::max(v, max_value_);
      ++calls_;
    }
  };

  std::map<Object*, Statistics> stats_data_;
}





void Model::set_gather_statistics(bool tf) {
  gather_statistics_=tf;
}


void Model::show_statistics_summary(std::ostream &out) const {
  out << "ScoreStates: running_time_before running_time_after\n";
  for (ScoreStateConstIterator it= score_states_begin();
       it != score_states_end(); ++it) {
    if (stats_data_.find(*it) != stats_data_.end()) {
      out << "  " << (*it)->get_name() << ": ";
      out << stats_data_[*it].total_time_/ stats_data_[*it].calls_
          << "s "
          << stats_data_[*it].total_time_after_/ stats_data_[*it].calls_
          << "s\n";
    }
  }

  out << "Restraints: running_time min_value max_value average_value\n";
  for (RestraintConstIterator it= restraints_begin();
       it != restraints_end(); ++it) {
    if (stats_data_.find(*it) != stats_data_.end()) {
      out << "  " << (*it)->get_name() << ": ";
      out << stats_data_[*it].total_time_/ stats_data_[*it].calls_
          << "s "
          << stats_data_[*it].min_value_ << " "
          << stats_data_[*it].max_value_ << " "
          << stats_data_[*it].total_value_/ stats_data_[*it].calls_ << "\n";
    }
  }
}


void Model::add_to_update_before_time(ScoreState *s, double t) const {
  ++stats_data_[s].calls_;
  stats_data_[s].total_time_+=t;
}
void Model::add_to_update_after_time(ScoreState *s, double t) const {
  stats_data_[s].total_time_after_+=t;
}
void Model::add_to_restraint_evaluate(Restraint *r, double t,
                                      double score) const {
  stats_data_[r].total_time_+=t;
  ++stats_data_[r].calls_;
  stats_data_[r].min_value_= std::min(score, stats_data_[r].min_value_);
  stats_data_[r].max_value_= std::max(score, stats_data_[r].max_value_);
  stats_data_[r].total_value_+=score;
}



IMP_END_NAMESPACE
