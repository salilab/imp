/**
 *  \file Model.cpp \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/Model.h"
#include "IMP/Particle.h"
#include "IMP/log.h"
#include "IMP/Restraint.h"
#include "IMP/DerivativeAccumulator.h"
#include "IMP/ScoreState.h"
#include <boost/timer.hpp>

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


//! Constructor
Model::Model()
{
  iteration_ = 0;
  next_particle_index_=0;
  cur_stage_=NOT_EVALUATING;
  incremental_update_=false;
  first_incremental_=true;
  gather_statistics_=false;
}


//! Destructor
Model::~Model()
{
  IMP_CHECK_OBJECT(this);
  for (ParticleIterator it= particles_begin();
       it != particles_end(); ++it) {
    (*it)->ps_->model_ = NULL;
    internal::unref(*it);
  }
}

IMP_LIST_IMPL(Model, Restraint, restraint, Restraint*,
              Restraints,
              {obj->set_model(this);
                first_incremental_=true;},,
              {obj->set_model(NULL);});

IMP_LIST_IMPL(Model, ScoreState, score_state, ScoreState*,
              ScoreStates,
              {obj->set_model(this);},,
              {obj->set_model(NULL);});


FloatRange Model::get_range(FloatKey k) const {
  IMP_CHECK_OBJECT(this);
  if (ranges_.find(k) != ranges_.end()) {
    return ranges_.find(k)->second;
  } else {
    FloatRange r(std::numeric_limits<Float>::max(),
                -std::numeric_limits<Float>::max());
    for (ParticleConstIterator it= particles_begin();
         it != particles_end(); ++it) {
      if ((*it)->has_attribute(k)) {
        Float v= (*it)->get_value(k);
        r.first = std::min(r.first, v);
        r.second= std::max(r.second, v);
      }
    }
    IMP_LOG(TERSE, "Range for attribute " << k << " is " << r.first
            << " to " << r.second << std::endl);
    return r;
  }
}

void Model::before_evaluate() const {
  IMP_LOG(TERSE,
          "Begin update ScoreStates " << std::endl);
  cur_stage_= BEFORE_EVALUATE;
  boost::timer timer;
  for (ScoreStateConstIterator it = score_states_begin();
       it != score_states_end(); ++it) {
    IMP_CHECK_OBJECT(*it);
    IMP_LOG(TERSE, "Updating " << (*it)->get_name() << std::endl);
    if (gather_statistics_) timer.restart();
    (*it)->before_evaluate(iteration_);
    if (gather_statistics_) {
      stats_data_[*it].update_state_before(timer.elapsed());
    }
    IMP_LOG(VERBOSE, "." << std::flush);
  }
  IMP_LOG(TERSE, "End update ScoreStates." << std::endl);
}

void Model::after_evaluate(bool calc_derivs) const {
  IMP_LOG(TERSE,
          "Begin after_evaluate of ScoreStates " << std::endl);
  DerivativeAccumulator accum;
  cur_stage_= AFTER_EVALUATE;
  boost::timer timer;
  for (ScoreStateConstIterator it = score_states_begin();
       it != score_states_end(); ++it) {
    IMP_CHECK_OBJECT(*it);
    IMP_LOG(TERSE, "Updating " << (*it)->get_name() << std::endl);
    if (gather_statistics_) timer.restart();
    (*it)->after_evaluate(iteration_, (calc_derivs ? &accum : NULL));
    if (gather_statistics_) {
      stats_data_[*it].update_state_after(timer.elapsed());
    }
    IMP_LOG(VERBOSE, "." << std::flush);
  }
  IMP_LOG(TERSE, "End after_evaluate of ScoreStates." << std::endl);
}

void Model::zero_derivatives(bool st) const {
  for (ParticleConstIterator pit = particles_begin();
       pit != particles_end(); ++pit) {
    (*pit)->zero_derivatives();
    if (st) {
      (*pit)->get_prechange_particle()->zero_derivatives();
    }
  }
}

double Model::do_evaluate(bool calc_derivs) const {
  // evaluate all of the active restraints to get score (and derivatives)
  // for current state of the model
  if (calc_derivs) {
    zero_derivatives();
  }
  double score= do_evaluate_restraints(calc_derivs, ALL, false);
  return score;
}


double Model::do_evaluate_restraints(bool calc_derivs,
                                     WhichRestraints incremental_restraints,
                                     bool incremental_evaluation) const {
  IMP_IF_LOG(TERSE) {
    std::string which;
    if (incremental_restraints== ALL) which="all";
    else if (incremental_restraints== NONINCREMENTAL) which="non-incremental";
    else which = "incremental";
    IMP_LOG(TERSE,
            "Begin " << (incremental_evaluation ? "incremental-":"")
            << "evaluate of "
            << which << " restraints "
            << (calc_derivs?"with derivatives":"without derivatives")
            << std::endl);
  }
  double score=0;
  DerivativeAccumulator accum;
  boost::timer timer;
  for (RestraintConstIterator it= restraints_begin();
       it != restraints_end(); ++it) {
    double value;
    if (gather_statistics_) timer.restart();
    if ((*it)->get_is_incremental()
        && incremental_restraints != NONINCREMENTAL) {
      if (incremental_evaluation) {
        value=(*it)->incremental_evaluate(calc_derivs? &accum:NULL);
        IMP_LOG(TERSE, (*it)->get_name() << " score is " << value << std::endl);
      } else {
        value=(*it)->evaluate(calc_derivs? &accum:NULL);
        IMP_LOG(TERSE, (*it)->get_name() << " score is " << value << std::endl);
      }
    } else if (!(*it)->get_is_incremental()
               && incremental_restraints != INCREMENTAL) {
      value=(*it)->evaluate(calc_derivs? &accum:NULL);
      IMP_LOG(TERSE, (*it)->get_name()<<  " score is " << value << std::endl);
    }
    if (gather_statistics_) {
      stats_data_[*it].update_restraint(timer.elapsed(), value);
    }
    score+= value;
  }
  IMP_LOG(TERSE, "End evaluate restraints." << std::endl);
  return score;
}



double Model::do_evaluate_incremental(bool calc_derivs) const {
  double score = 0.0;

  if (calc_derivs) zero_derivatives(first_incremental_);

  score+= do_evaluate_restraints(calc_derivs, INCREMENTAL, !first_incremental_);
  if (calc_derivs) {
    for (ParticleConstIterator pit = particles_begin();
         pit != particles_end(); ++pit) {
      (*pit)->move_derivatives_to_shadow();
    }
  }

  score+=do_evaluate_restraints(calc_derivs, NONINCREMENTAL, false);

  for (ParticleConstIterator pit = particles_begin();
       pit != particles_end(); ++pit) {
    if (calc_derivs) (*pit)->accumulate_derivatives_from_shadow();
    (*pit)->set_is_not_changed();
  }
  return score;
}


namespace {
  template <class T, int V>
  struct SetIt {
    T *t_;
    SetIt(T *t): t_(t){}
    ~SetIt() {
      *t_= T(V);
    }
  };
}

Float Model::evaluate(bool calc_derivs)
{
  // make sure stage is restored on an exception
  SetIt<Stage, NOT_EVALUATING> reset(&cur_stage_);
  IMP_OBJECT_LOG;
  IMP_CHECK_OBJECT(this);
  IMP_LOG(TERSE,
          "Begin Model::evaluate "
          << (get_is_incremental() && first_incremental_ ? "first ":"")
          << (get_is_incremental()?"incremental":"")
          << std::endl);

  if (incremental_update_ && !last_had_derivatives_ && calc_derivs) {
    first_incremental_=true;
  }

  last_had_derivatives_= calc_derivs;

  before_evaluate();

  cur_stage_= EVALUATE;
  double score;
  if (get_is_incremental()) {
    score= do_evaluate_incremental(calc_derivs);
    first_incremental_=false;
  } else {
    score= do_evaluate(calc_derivs);
  }

  after_evaluate(calc_derivs);

  IMP_LOG(TERSE, "End Model::evaluate. Final score: " << score << std::endl);
  cur_stage_=NOT_EVALUATING;
  ++iteration_;
  return score;
}


void Model::set_is_incremental(bool tf) {
  DerivativeAccumulator da;
  if (tf && !get_is_incremental()) {
    first_incremental_=true;
    for (ParticleIterator it= particles_begin(); it != particles_end(); ++it) {
      (*it)->setup_incremental();
    }
    // must be done in two passes so that all shadow particles are available
    // to copy the particles table.
    for (ParticleIterator it= particles_begin(); it != particles_end(); ++it) {
      (*it)->set_is_not_changed();
    }
  } else if (!tf && get_is_incremental()) {
    for (ParticleIterator it= particles_begin(); it != particles_end(); ++it) {
      (*it)->teardown_incremental();
    }
  }
  incremental_update_=tf;
}

void Model::show(std::ostream& out) const
{
  out << std::endl << std::endl;
  out << "Model:" << std::endl;

  get_version_info().show(out);

  out << get_number_of_particles() << " particles" << std::endl;
  out << get_number_of_restraints() << " restraints" << std::endl;
  out << get_number_of_score_states() << " score states" << std::endl;

  out << std::endl;
  IMP_CHECK_OBJECT(this);
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



IMP_END_NAMESPACE
