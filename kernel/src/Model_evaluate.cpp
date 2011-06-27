/**
 *  \file Model.cpp \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/Model.h"
#include "IMP/Particle.h"
#include "IMP/log.h"
#include "IMP/Restraint.h"
#include "IMP/DerivativeAccumulator.h"
#include "IMP/ScoreState.h"
#include <boost/timer.hpp>
#include "IMP/compatibility/set.h"
#include <numeric>


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
                  << " is not in the input particles list of "          \
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
                  << " is not in the input particles list of "          \
                  << (restraint)->get_name() << " but should be."       \
                  << "The list contains " << Particles(rpl)             \
                  << std::endl);                                        \
        throw InternalException("Invalid particle used");               \
      } catch (internal::WriteLockedParticleException e) {              \
        IMP_ERROR("Particle " << e.p_->get_name()                       \
                  << " is not in the output particles list of "         \
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
  compatibility::set<Object *> allowed_;
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
  compatibility::set<Object *> allowed_;
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
  CreateLogContext clc("update_score_states");
  {
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
      //IMP_LOG(VERBOSE, "." << std::flush);
    }
  }
}

void Model::after_evaluate(const ScoreStatesTemp &states,
                           bool calc_derivs) const {
  CreateLogContext clc("update_derivatives");
  {
    DerivativeAccumulator accum;
    cur_stage_= AFTER_EVALUATE;
    boost::timer timer;
    for (int i=states.size()-1; i>=0; --i) {
      ScoreState *ss= states[i];
      IMP_CHECK_OBJECT(ss);
      IMP_LOG(TERSE, "Post updating \"" << ss->get_name() << "\"" << std::endl);
      if (gather_statistics_) timer.restart();
      WRAP_UPDATE_CALL(ss, ss->after_evaluate(calc_derivs?&accum:NULL), true);
      if (gather_statistics_) {
        add_to_update_after_time(ss, timer.elapsed());
      }
      //IMP_LOG(VERBOSE, "." << std::flush);
    }
  }
}

void Model::zero_derivatives() const {
  for (ParticleConstIterator pit = particles_begin();
       pit != particles_end(); ++pit) {
    (*pit)->zero_derivatives();
  }
}

Floats Model::do_evaluate_restraints(const RestraintsTemp &restraints,
                                     const std::vector<double> &weights,
                                     bool calc_derivs,
                                     bool if_good) {
  IMP_FUNCTION_LOG;
  Floats ret;
  double remaining=get_maximum_score();
  boost::timer timer;
  const unsigned int rsz=restraints.size();
  for (unsigned int i=0; i< rsz; ++i) {
    double value=0;
    DerivativeAccumulator accum(weights[i]);
    if (gather_statistics_) timer.restart();
    if (if_good) {
      double max=std::min(remaining,
                          restraints[i]->get_maximum_score());
      WRAP_EVALUATE_CALL(restraints[i],
                         value=
                   restraints[i]->unprotected_evaluate_if_good(calc_derivs?
                                                              &accum:NULL,
                                                                   max));
    } else {
      WRAP_EVALUATE_CALL(restraints[i],
                         value=
                         restraints[i]->unprotected_evaluate(calc_derivs?
                                                             &accum:NULL));
    }
    double wvalue= weights[i]*value;
    remaining-=wvalue;
    IMP_LOG(TERSE, restraints[i]->get_name()<<  " score is "
              << wvalue << std::endl);
    if (gather_statistics_) {
      add_to_restraint_evaluate(restraints[i], timer.elapsed(), wvalue);
    }
    if (value > restraints[i]->get_maximum_score()) {
      has_good_score_=false;
    }
    ret.push_back(wvalue);
  }
  if (remaining<0) {
    has_good_score_=false;
  }
  return ret;
}



void Model::validate_computed_derivatives() const {
  for (ParticleConstIterator it= particles_begin();
       it != particles_end(); ++it) {
    Particle *p= *it;
    p->validate_float_derivatives();
  }
}






Floats Model::do_evaluate(const RestraintsTemp &restraints,
                          const std::vector<double> &weights,
                          const ScoreStatesTemp &states,
                          bool calc_derivs,
                          bool if_good) {
  // make sure stage is restored on an exception
  SetIt<Stage, NOT_EVALUATING> reset(&cur_stage_);
  IMP_CHECK_OBJECT(this);
  IMP_LOG(VERBOSE, "On restraints " << Restraints(restraints)
          << " and score states " << ScoreStates(states)
          << std::endl);

  before_evaluate(states);

  cur_stage_= EVALUATE;
  if (calc_derivs) {
    zero_derivatives();
  }
  std::vector<double> ret= do_evaluate_restraints(restraints, weights,
                                                  calc_derivs, if_good);

  after_evaluate(states, calc_derivs);

  // validate derivatives
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    if (calc_derivs) {
      validate_computed_derivatives();
    }
  }
  IMP_LOG(TERSE, "Final score: "
          << std::accumulate(ret.begin(), ret.end(), 0.0) << std::endl);
  cur_stage_=NOT_EVALUATING;
  ++eval_count_;
  first_call_=false;
  return ret;
}


IMP_END_NAMESPACE
