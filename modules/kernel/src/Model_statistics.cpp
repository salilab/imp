/**
 *  \file Model.cpp \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
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
#include <boost/format.hpp>
#include <set>



IMP_BEGIN_NAMESPACE

void Model::set_gather_statistics(bool tf) {
  gather_statistics_=tf;
}


void Model::show_restraint_time_statistics(std::ostream &out) const {
  out << "Restraints: running_time [min_value max_value] "
      << "average_value last_value\n";
  RestraintsTemp r= IMP::get_restraints(restraints_begin(), restraints_end());
  for (RestraintsTemp::const_iterator it= r.begin();
       it != r.end(); ++it) {
    if (stats_data_.find(*it) != stats_data_.end()) {
      out << "  " << (*it)->get_name() << ": ";
      out << stats_data_[*it].total_time_/ stats_data_[*it].calls_
          << "s\n";
    } else {
      out << "  " << (*it)->get_name() << ":\n";
    }
  }
}
void Model::show_score_state_time_statistics(std::ostream &out) const {
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
}
void Model::show_all_statistics(std::ostream &out) const {
  show_restraint_score_statistics(out);
  show_restraint_time_statistics(out);
  show_score_state_time_statistics(out);
}

void Model::show_restraint_score_statistics(std::ostream &out) const {
  boost::format format("%1%: [%15t%2% %30t%3%] %45t%4% %60t%5% %70t%6%\n");
  out << format
    % "Restraints" % "min_value" % "max_value" % "average_value"
    %"limit" % "last_value";
  RestraintsTemp r= IMP::get_restraints(restraints_begin(), restraints_end());
  for (RestraintsTemp::const_iterator it= r.begin();
       it != r.end(); ++it) {
    if (stats_data_.find(*it) != stats_data_.end()) {
      out << format % (std::string("  ")+ (*it)->get_name())
        % stats_data_[*it].min_value_
        % stats_data_[*it].max_value_
        % (stats_data_[*it].total_value_/ stats_data_[*it].calls_)
        % (*it)->get_maximum_score()
        % stats_data_[*it].last_value_;
    } else {
      out << "  " << (*it)->get_name() << ":\n";
    }
  }
}

RestraintStatistics Model::get_restraint_statistics(Restraint *r) const {
  if (stats_data_.find(r) == stats_data_.end()) {
    IMP_THROW("Invalid restraint", base::ValueException);
  }
  RestraintStatistics ret;
  base::map<Object*, Statistics>::const_iterator
    it= stats_data_.find(r);
  ret.set_minimum_score(it->second.min_value_);
  ret.set_maximum_score(it->second.max_value_);
  ret.set_average_score(it->second.total_value_/ it->second.calls_);
  ret.set_last_score(it->second.last_value_);
  ret.set_average_time(it->second.total_time_/ it->second.calls_);
  return ret;
}


void Model::clear_all_statistics() {
  for (ScoreStateConstIterator it= score_states_begin();
       it != score_states_end(); ++it) {
    stats_data_.erase(*it);
  }
  RestraintsTemp r= IMP::get_restraints(restraints_begin(), restraints_end());
  for (RestraintsTemp::const_iterator it= r.begin();
       it != r.end(); ++it) {
    stats_data_.erase(*it);
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
  stats_data_[r].last_value_=score;
  stats_data_[r].total_value_+=score;
}



IMP_END_NAMESPACE
