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
#include <boost/timer.hpp>
#include <set>


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

#if IMP_BUILD < IMP_FAST
#define WRAP_UPDATE_CALL(restraint, expr, exchange)                     \
  {                                                                     \
    IMP_IF_CHECK(first_call_ && USAGE_AND_INTERNAL) {                   \
      ParticlesTemp rpl;                                                \
      if (!exchange) rpl= (restraint)->get_input_particles();           \
      else rpl= (restraint)->get_output_particles();                    \
      ParticlesTemp wpl;                                                \
      if (!exchange) wpl= (restraint)->get_output_particles();          \
      else wpl= (restraint)->get_input_particles();                     \
      wpl.insert(wpl.end(), rpl.begin(), rpl.end());                    \
      if (exchange) {                                                   \
        rpl.insert(rpl.end(), wpl.begin(), wpl.end());                  \
      }                                                                 \
      ContainersTemp cpl= (restraint)->get_input_containers();          \
      {for (unsigned int i=0; i < cpl.size(); ++i) {                    \
        if (dynamic_cast<Particle*>(cpl[i])) {                          \
          if (exchange) {                                               \
            wpl.push_back(dynamic_cast<Particle*>(cpl[i]));             \
          } else {                                                      \
            rpl.push_back(dynamic_cast<Particle*>(cpl[i]));             \
          }                                                             \
        }                                                               \
        }}                                                              \
      ContainersTemp cpo= (restraint)->get_output_containers();         \
      {for (unsigned int i=0; i < cpo.size(); ++i) {                    \
        if (dynamic_cast<Particle*>(cpo[i])) {                          \
          if (!exchange) {                                              \
            wpl.push_back(dynamic_cast<Particle*>(cpo[i]));             \
          } else {                                                      \
            rpl.push_back(dynamic_cast<Particle*>(cpo[i]));             \
          }                                                             \
        }                                                               \
        }}                                                              \
      internal::ReadLock rl(particles_begin(), particles_end(),         \
                            rpl.begin(), rpl.end());                    \
      internal::WriteLock wl(particles_begin(), particles_end(),        \
                             wpl.begin(), wpl.end());                   \
      try {                                                             \
        expr;                                                           \
      } catch (internal::ReadLockedParticleException &e) {              \
        std::ostringstream oss;                                         \
        for (unsigned int i=0; i< rpl.size(); ++i) {                    \
          oss << rpl[i]->get_name() << " ";                             \
        }                                                               \
        IMP_ERROR("Particle " << e.p_->get_name()                       \
                  << " is not in the read particles list of "           \
                  << (restraint)->get_name() << " but should be. "      \
                  << "The list contains " << oss.str()                  \
                  << std::endl);                                        \
        throw InternalException("Invalid particle used ");              \
      } catch (internal::WriteLockedParticleException &e) {             \
        std::ostringstream oss;                                         \
        for (unsigned int i=0; i< wpl.size(); ++i) {                    \
          oss << wpl[i]->get_name() << " ";                             \
        }                                                               \
        IMP_ERROR("Particle " << e.p_->get_name()                       \
                  << " is not in the write particles list of "          \
                  << (restraint)->get_name() << " but should be."       \
                  << "The list contains " << oss.str()                  \
                  << std::endl);                                        \
        throw InternalException("Invalid particle used");               \
      }                                                                 \
    } else {                                                            \
      expr;                                                             \
    }                                                                   \
  }

#define WRAP_EVALUATE_CALL(restraint, expr)                             \
  {                                                                     \
    IMP_IF_CHECK(first_call_ && USAGE_AND_INTERNAL) {                   \
      ParticlesTemp rpl= (restraint)->get_input_particles();            \
      ContainersTemp cpl= (restraint)->get_input_containers();          \
      {for (unsigned int i=0; i < cpl.size(); ++i) {                    \
        if (dynamic_cast<Particle*>(cpl[i])) {                          \
          rpl.push_back(dynamic_cast<Particle*>(cpl[i]));               \
        }                                                               \
        }}                                                              \
      internal::ReadLock rl(particles_begin(), particles_end(),         \
                            rpl.begin(), rpl.end());                    \
      internal::WriteLock wl(particles_begin(), particles_end(),        \
                             rpl.begin(), rpl.end());                   \
      try {                                                             \
        expr;                                                           \
      } catch (internal::ReadLockedParticleException e) {               \
        IMP_ERROR("Particle " << e.p_->get_name()                       \
                  << " is not in the read particles list of "           \
                  << (restraint)->get_name() << " but should be."       \
                  << "The list contains " << Particles(rpl)             \
                  << std::endl);                                        \
        throw InternalException("Invalid particle used");               \
      } catch (internal::WriteLockedParticleException e) {              \
        IMP_ERROR("Particle " << e.p_->get_name()                       \
                  << " is not in the write particles list of "          \
                  << (restraint)->get_name() << " but should be."       \
                  << "The list contains " << Particles(rpl)             \
                  << std::endl);                                        \
        throw InternalException("Invalid particle used");               \
      }                                                                 \
    } else {                                                            \
      expr;                                                             \
    }                                                                   \
  }
#else
#define WRAP_UPDATE_CALL(restraint, expr, exchange) expr
#define WRAP_EVALUATE_CALL(restraint, expr) expr
#endif


IMP_BEGIN_INTERNAL_NAMESPACE

struct ReadLock{
  Particles p_;
  std::set<Object *> allowed_;
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
  Particles p_;
  std::set<Object *> allowed_;
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


void Model::before_evaluate(const ScoreStatesTemp &states) const {
  IMP_USAGE_CHECK(cur_stage_== NOT_EVALUATING,
                  "Can only call Model::before_evaluate() when not evaluating");
  IMP_LOG(TERSE,
          "Begin update ScoreStates " << std::endl);
  {
    IncreaseIndent ii;
    cur_stage_= BEFORE_EVALUATE;
    boost::timer timer;
    for (unsigned int i=0; i< states.size(); ++i) {
      ScoreState *ss= states[i];
      IMP_CHECK_OBJECT(ss);
      IMP_LOG(TERSE, "Updating \"" << ss->get_name() << "\"" << std::endl);
      if (gather_statistics_) timer.restart();
      WRAP_UPDATE_CALL(ss, ss->before_evaluate(), false);
      if (gather_statistics_) {
        add_to_update_before_time(ss, timer.elapsed());
      }
      IMP_LOG(VERBOSE, "." << std::flush);
    }
  }
  IMP_LOG(TERSE, "End update ScoreStates." << std::endl);
}

void Model::after_evaluate(const ScoreStatesTemp &states,
                           bool calc_derivs) const {
  IMP_LOG(TERSE,
          "Begin after_evaluate of ScoreStates " << std::endl);
  {
    IncreaseIndent ii;
    DerivativeAccumulator accum;
    cur_stage_= AFTER_EVALUATE;
    boost::timer timer;
    for (int i=states.size()-1; i>=0; --i) {
      ScoreState *ss= states[i];
      IMP_CHECK_OBJECT(ss);
      IMP_LOG(TERSE, "Updating \"" << ss->get_name() << "\"" << std::endl);
      if (gather_statistics_) timer.restart();
      WRAP_UPDATE_CALL(ss, ss->after_evaluate(calc_derivs?&accum:NULL), true);
      if (gather_statistics_) {
        add_to_update_after_time(ss, timer.elapsed());
      }
      IMP_LOG(VERBOSE, "." << std::flush);
    }
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

double Model::do_evaluate_restraints(const RestraintsTemp &restraints,
                                     const std::vector<double> &weights,
                                     bool calc_derivs,
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
  boost::timer timer;
  for (unsigned int i=0; i< restraints.size(); ++i) {
    double value;
    if (gather_statistics_) timer.restart();
    if (restraints[i]->get_is_incremental()
        && incremental_restraints != NONINCREMENTAL) {
      if (incremental_evaluation) {
        DerivativeAccumulator accum(weights[i]);
        WRAP_EVALUATE_CALL(restraints[i],
                           value=weights[i]*restraints[i]
                           ->unprotected_incremental_evaluate(calc_derivs?
                                                              &accum:NULL));
        IMP_LOG(TERSE, restraints[i]->get_name() << " score is "
                << value << std::endl);
      } else {
        DerivativeAccumulator accum(weights[i]);
        WRAP_EVALUATE_CALL(restraints[i],
                           value=weights[i]
                           *restraints[i]->unprotected_evaluate(calc_derivs?
                                                                &accum:NULL));
        IMP_LOG(TERSE, restraints[i]->get_name() << " score is "
                << value << std::endl);
      }
      if (gather_statistics_) {
        add_to_restraint_evaluate(restraints[i], timer.elapsed(), value);
      }
      score+= value;
    } else if (!restraints[i]->get_is_incremental()
               && incremental_restraints != INCREMENTAL) {
      DerivativeAccumulator accum(weights[i]);
      WRAP_EVALUATE_CALL(restraints[i],
                         value=weights[i]
                         *restraints[i]->unprotected_evaluate(calc_derivs?
                                                           &accum:NULL));
      IMP_LOG(TERSE, restraints[i]->get_name()<<  " score is "
              << value << std::endl);
      if (gather_statistics_) {
        add_to_restraint_evaluate(restraints[i], timer.elapsed(), value);
      }
      score+= value;
    }
  }
  IMP_LOG(TERSE, "End evaluate restraints." << std::endl);
  return score;
}



void Model::validate_incremental_evaluate(const RestraintsTemp &restraints,
                                          const std::vector<double> &weights,
                                          bool calc_derivs,
                                          double score) {
  IMP_LOG(TERSE, "Begin checking incremental evaluation"<<std::endl);
  {
    IncreaseIndent ii;
    std::vector<internal::ParticleStorage::DerivativeTable>
      derivs;
    derivs.reserve(get_number_of_particles());
    for (ParticleIterator it= particles_begin();
         it != particles_end(); ++it) {
      derivs.push_back((*it)->ps_->derivatives_);
      (*it)->zero_derivatives();
    }
    bool ogather_stats=gather_statistics_;
    gather_statistics_=false;
    double nscore=
      do_evaluate_restraints(restraints, weights,
                             calc_derivs, ALL, false);
    gather_statistics_= ogather_stats;
    if (std::abs(nscore -score)
        > .001+.1*std::abs(nscore+score)) {
      if (gather_statistics_) {
        std::cerr << "Incremental:\n";
        show_restraint_score_statistics(std::cerr);
        do_evaluate_restraints(restraints, weights,
                               calc_derivs, ALL, false);
        std::cerr << "Non-incremental:\n";
        show_restraint_score_statistics(std::cerr);
      }
      IMP_FAILURE("Incremental and non-incremental evaluation "
                  << "do not agree."
                  << " Incremental gets " << score
                  << " but non-incremental "
                  << "gets " << nscore);
    }
    if (calc_derivs) {
      unsigned int i=0;
      for (ParticleIterator it= particles_begin();
           it != particles_end(); ++it) {
        for (unsigned int j=0; j< derivs[i].get_length(); ++j) {
          IMP_INTERNAL_CHECK(std::abs(derivs[i].get(j)
                                      -(*it)->ps_->derivatives_.get(j))
                             < .01
                             + .01*std::abs(derivs[i].get(j)
                                            +(*it)->ps_->derivatives_.get(j)),
                             "Derivatives do not match in incremental "
                             << "vs non-incremental " <<
                             "for particle " << (*it)->get_name()
                             << " on attribute "
                             << FloatKey(j) << ". Incremental was "
                             << derivs[i].get(j)
                             << " where as regular was "
                             << (*it)->ps_->derivatives_.get(j)
                             << " particle is "
                             << *(*it));
        }
        (*it)->ps_->derivatives_=derivs[i];
        ++i;
      }
      IMP_INTERNAL_CHECK(i== derivs.size(), "Number of particles changed.");
    }
  }
  IMP_LOG(TERSE, "End checking incremental evaluation"<<std::endl);
}

void Model::validate_computed_derivatives() const {
  for (ParticleConstIterator it= particles_begin();
       it != particles_end(); ++it) {
    Particle *p= *it;
    p->validate_float_derivatives();
  }
}






double Model::do_evaluate(const RestraintsTemp &restraints,
                          const std::vector<double> &weights,
                          const ScoreStatesTemp &states,
                          bool calc_derivs) {
  // make sure stage is restored on an exception
  SetIt<Stage, NOT_EVALUATING> reset(&cur_stage_);
  IMP_CHECK_OBJECT(this);
  IMP_LOG(TERSE,
          "Begin Model::evaluate "
          << (get_is_incremental() && first_incremental_ ? "first ":"")
          << (get_is_incremental()?"incremental":"")
          << std::endl);
  IMP_LOG(VERBOSE, "On restraints " << Restraints(restraints)
          << " and score states " << ScoreStates(states)
          << std::endl);

  if (incremental_update_ && !last_had_derivatives_ && calc_derivs) {
    first_incremental_=true;
  }

  last_had_derivatives_= calc_derivs;

  before_evaluate(states);

  cur_stage_= EVALUATE;
  double score;
  if (get_is_incremental()) {
    score = 0.0;
    if (calc_derivs) zero_derivatives(first_incremental_);
    score+= do_evaluate_restraints(restraints, weights,
                                   calc_derivs, INCREMENTAL,
                                   !first_incremental_);
    if (calc_derivs) {
      for (ParticleConstIterator pit = particles_begin();
           pit != particles_end(); ++pit) {
        (*pit)->move_derivatives_to_shadow();
      }
    }
    score+=do_evaluate_restraints(restraints, weights,
                                  calc_derivs, NONINCREMENTAL, false);
    if (calc_derivs) {
      for (ParticleConstIterator pit = particles_begin();
           pit != particles_end(); ++pit) {
        (*pit)->accumulate_derivatives_from_shadow();
      }
    }
    first_incremental_=false;
    IMP_IF_CHECK(USAGE_AND_INTERNAL) {
      validate_incremental_evaluate(restraints, weights, calc_derivs, score);
    }
  } else {
    if (calc_derivs) {
      zero_derivatives();
    }
    score= do_evaluate_restraints(restraints, weights,
                                  calc_derivs, ALL, false);
  }

  after_evaluate(states, calc_derivs);

  // validate derivatives
  if (calc_derivs) {
    validate_computed_derivatives();
  }
  //if (get_is_incremental()) {
  IMP_LOG(TERSE, "Backing up changed particles" << std::endl);
  for (ParticleConstIterator pit = particles_begin();
       pit != particles_end(); ++pit) {
    (*pit)->set_is_not_changed();
  }
  //}
  IMP_LOG(TERSE, "End Model::evaluate. Final score: " << score << std::endl);
  cur_stage_=NOT_EVALUATING;
  ++eval_count_;
  return score;
}


IMP_END_NAMESPACE
