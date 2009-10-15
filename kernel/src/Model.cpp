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
#include <set>

#if IMP_BUILD < IMP_FAST
#define WRAP_CALL(restraint, expr)                                      \
  {                                                                     \
    IMP_IF_CHECK(USAGE_AND_INTERNAL) {                                  \
      ParticlesTemp rpl= (restraint)->get_read_particles();             \
      internal::ReadLock rl(particles_begin(), particles_end(),         \
                            rpl.begin(), rpl.end());                    \
      ParticlesTemp wpl= (restraint)->get_write_particles();            \
      internal::WriteLock wl(particles_begin(), particles_end(),        \
                             wpl.begin(), wpl.end());                   \
      try {                                                             \
        expr;                                                           \
      } catch (internal::ReadLockedParticleException e) {               \
        IMP_ERROR("Particle " << e.p_->get_name()                       \
                  << " is not in the read particles list of "           \
                  << (*it)->get_name() << " but should be.");           \
        throw InvalidStateException("Invalid particle used");           \
      } catch (internal::WriteLockedParticleException e) {              \
        IMP_ERROR("Particle " << e.p_->get_name()                       \
                  << " is not in the write particles list of "          \
                  << (*it)->get_name() << " but should be.");           \
        throw InvalidStateException("Invalid particle used");           \
      }                                                                 \
    } else {                                                            \
      expr;                                                             \
    }                                                                   \
  }
#else
#define WRAP_CALL(restraint, expr) expr
#endif


IMP_BEGIN_INTERNAL_NAMESPACE
struct ReadLock{
  ParticlesTemp p_;
  std::set<Particle *> allowed_;
public:
  template <class It, class It1>
  ReadLock(It1 pa, It1 pb,
           It ab, It ae): p_(pa, pb),
                          allowed_(ab, ae){
#if IMP_BUILD < IMP_FAST
    for (unsigned int i=0; i< p_.size(); ++i) {
      if (allowed_.find(p_[i]) == allowed_.end()) {
        p_[i]->ps_->read_locked_=true;
      }
    }
#endif
  }
  ~ReadLock() {
#if IMP_BUILD < IMP_FAST
    for (unsigned int i=0; i< p_.size(); ++i) {
      p_[i]->ps_->read_locked_=false;
    }
#endif
  }
};


struct WriteLock{
  ParticlesTemp p_;
  std::set<Particle *> allowed_;
public:
  template <class It, class It1>
  WriteLock(It1 pa, It1 pb,
           It ab, It ae): p_(pa, pb),
                          allowed_(ab, ae){
#if IMP_BUILD < IMP_FAST
    for (unsigned int i=0; i< p_.size(); ++i) {
      if (allowed_.find(p_[i]) == allowed_.end()) {
        p_[i]->ps_->write_locked_=true;
      }
    }
#endif
  }
  ~WriteLock() {
#if IMP_BUILD < IMP_FAST
    for (unsigned int i=0; i< p_.size(); ++i) {
      p_[i]->ps_->write_locked_=false;
    }
#endif
  }
};


IMP_END_INTERNAL_NAMESPACE

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
  score_states_ordered_=false;
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
              {IMP_INTERNAL_CHECK(cur_stage_== NOT_EVALUATING,
           "The set of restraints cannot be changed during evaluation.");
                obj->set_model(this);
                first_incremental_=true;},,
              {obj->set_model(NULL);});

IMP_LIST_IMPL(Model, ScoreState, score_state, ScoreState*,
              ScoreStates,
              {IMP_INTERNAL_CHECK(cur_stage_== NOT_EVALUATING,
           "The set of score states cannot be changed during evaluation.");
                obj->set_model(this);
                score_states_ordered_=false;
              },,
              {obj->set_model(NULL);});


void Model::order_score_states() {
  // go away once the ordering below happens
  IMP_IF_CHECK(USAGE) {
    std::set<Particle*> read_particles;
    for (ScoreStateConstIterator it = score_states_begin();
         it != score_states_end(); ++it) {
      ParticlesTemp wp= (*it)->get_write_particles();
      for (unsigned int i=0; i< wp.size(); ++i) {
        if (read_particles.find(wp[i]) != read_particles.end()) {
          IMP_WARN("Particle " << wp[i]->get_name()
                   << " was changed by score state "
                   << (*it)->get_name()
                   << " after it was read by another score state."
                   << " This may result in score states not being"
                   << " updated properly. Eventually they will be"
                   << " reordered automatically, but for now "
                   << "you have to do it yourself.");
        }
      }
      ParticlesTemp rp= (*it)->get_read_particles();
      read_particles.insert(rp.begin(), rp.end());
    }
  }

  ScoreStates new_order;
  new_order.reserve(get_number_of_score_states());
  /*
    generate bipartite graph with write_ss, read_ss
    for each writer, for each particle it writes,
    connect the writer to all readers of that particle

    iteratively chose a reader with no in edges,
    remove it and its writer and all edges

    produce error if anything is left
   */
  //std::map<Particle*, std::vector<Node> > readers;
  //std::map<ScoreState*, Node> reader_map, writer_map;
  for (ScoreStateIterator it= score_states_begin();
       it != score_states_end(); ++it) {
    // reader_map[*it]= create_node();
    // writer_map[*it]= create_node();
  }
  for (ScoreStateIterator it= score_states_begin();
       it != score_states_end(); ++it) {
    ParticlesTemp read= (*it)->get_read_particles();
    for (unsigned int i=0; i< read.size(); ++i) {
    }
  }
  score_states_ordered_=true;
}

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
    WRAP_CALL(*it, (*it)->before_evaluate(iteration_));
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
    WRAP_CALL(*it, (*it)->after_evaluate(iteration_,
                                         (calc_derivs ? &accum : NULL)));
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
        WRAP_CALL(*it,
                  value=(*it)->incremental_evaluate(calc_derivs? &accum:NULL));
        IMP_LOG(TERSE, (*it)->get_name() << " score is " << value << std::endl);
      } else {
        WRAP_CALL(*it,
                  value=(*it)->evaluate(calc_derivs? &accum:NULL));
        IMP_LOG(TERSE, (*it)->get_name() << " score is " << value << std::endl);
      }
      if (gather_statistics_) {
        stats_data_[*it].update_restraint(timer.elapsed(), value);
      }
      score+= value;
    } else if (!(*it)->get_is_incremental()
               && incremental_restraints != INCREMENTAL) {
      WRAP_CALL(*it,
                value=(*it)->evaluate(calc_derivs? &accum:NULL));
      IMP_LOG(TERSE, (*it)->get_name()<<  " score is " << value << std::endl);
      if (gather_statistics_) {
        stats_data_[*it].update_restraint(timer.elapsed(), value);
      }
      score+= value;
    }
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
  if (!score_states_ordered_) order_score_states();
  // validate values
  {
    std::string message;
    for (ParticleIterator it= particles_begin();
         it != particles_end(); ++it) {
      Particle *p= *it;
      for (Particle::FloatKeyIterator kit= p->float_keys_begin();
           kit != p->float_keys_end(); ++kit) {
        double d= p->get_value(*kit);
        if (is_nan(d) || std::abs(d) > std::numeric_limits<double>::max()) {
          IMP_IF_CHECK(USAGE) {
            std::ostringstream oss;
            oss << message << "Particle " << p->get_name()
                << " attribute " << *kit << " has derivative of "
                << d << std::endl;
            message= oss.str();
          }
          IMP_IF_CHECK(NONE) {
            message= "Bad particle value";
          }
        }
      }
    }
    if (!message.empty()) {
      throw InvalidStateException(message.c_str());
    }
  }



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
    IMP_IF_CHECK(USAGE_AND_INTERNAL) {
      std::vector<internal::ParticleStorage::DerivativeTable>
        derivs;
      derivs.reserve(get_number_of_particles());
      for (ParticleIterator it= particles_begin();
           it != particles_end(); ++it) {
        derivs.push_back((*it)->ps_->derivatives_);
        (*it)->zero_derivatives();
      }
      double nscore= do_evaluate(calc_derivs);
      IMP_INTERNAL_CHECK(std::abs(nscore -score)
                         < .001+.1*std::abs(nscore+score),
                 "Incremental and non-incremental evaluation do not agree."
                 << " Incremental gets " << score << " but non-incremental "
                 << "gets " << nscore);
      if (calc_derivs) {
        unsigned int i=0;
        for (ParticleIterator it= particles_begin();
             it != particles_end(); ++it) {
          for (unsigned int j=0; j< derivs[i].get_length(); ++j) {
            IMP_INTERNAL_CHECK(std::abs(derivs[i].get(j)
                                -(*it)->ps_->derivatives_.get(j))
                       < .001 + .01*std::abs(derivs[i].get(j)
                                             +(*it)->ps_->derivatives_.get(j)),
                       "Derivatives do not match in incremental "
                       << "vs non-incremental " <<
                       "for particle " << (*it)->get_name() << " on attribute "
                       << FloatKey(j) << ". Incremental was "
                       << derivs[i].get(j)
                       << " where as regular was "
                       << (*it)->ps_->derivatives_.get(j));
          }
          (*it)->ps_->derivatives_=derivs[i];
          ++i;
        }
        IMP_INTERNAL_CHECK(i== derivs.size(), "Number of particles changed.");
      }
    }
  } else {
    score= do_evaluate(calc_derivs);
  }

  after_evaluate(calc_derivs);

  // validate derivatives
  {
    std::string message;
    for (ParticleIterator it= particles_begin();
         it != particles_end(); ++it) {
      Particle *p= *it;
      for (Particle::FloatKeyIterator kit= p->float_keys_begin();
           kit != p->float_keys_end(); ++kit) {
        double d= p->get_derivative(*kit);
        if (is_nan(d) || std::abs(d) > std::numeric_limits<double>::max()) {
          IMP_IF_CHECK(USAGE) {
            std::ostringstream oss;
            oss << message << "Particle " << p->get_name()
                << " attribute " << *kit << " has derivative of "
                << d << std::endl;
            message= oss.str();
          }
          IMP_IF_CHECK(NONE) {
            message= "Bad particle derivative";
          }
        }
      }
    }
    if (!message.empty()) {
      throw InvalidStateException(message.c_str());
    }
  }

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
