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

IMP_BEGIN_NAMESPACE

//! Constructor
Model::Model()
{
  iteration_ = 0;
  next_particle_index_=0;
  cur_stage_=NOT_EVALUATING;
  incremental_update_=false;
  first_incremental_=true;
}


//! Destructor
Model::~Model()
{
  IMP_CHECK_OBJECT(this);
  for (ParticleIterator it= particles_begin();
       it != particles_end(); ++it) {
    (*it)->model_ = NULL;
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
  for (ScoreStateConstIterator it = score_states_begin();
       it != score_states_end(); ++it) {
    IMP_CHECK_OBJECT(*it);
    IMP_LOG(TERSE, "Updating " << (*it)->get_name() << std::endl);
    (*it)->before_evaluate(iteration_);
    IMP_LOG(VERBOSE, "." << std::flush);
  }
  IMP_LOG(TERSE, "End update ScoreStates." << std::endl);
}

void Model::after_evaluate(bool calc_derivs) const {
  IMP_LOG(TERSE,
          "Begin after_evaluate of ScoreStates " << std::endl);
  DerivativeAccumulator accum;
  cur_stage_= AFTER_EVALUATE;
  for (ScoreStateConstIterator it = score_states_begin();
       it != score_states_end(); ++it) {
    IMP_CHECK_OBJECT(*it);
    IMP_LOG(TERSE, "Updating " << (*it)->get_name() << std::endl);
    (*it)->after_evaluate(iteration_, (calc_derivs ? &accum : NULL));
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
  for (RestraintConstIterator it= restraints_begin();
       it != restraints_end(); ++it) {
    if ((*it)->get_is_incremental()
        && incremental_restraints != NONINCREMENTAL) {
      if (incremental_evaluation) {
        double t=(*it)->incremental_evaluate(calc_derivs? &accum:NULL);
        IMP_LOG(TERSE, (*it)->get_name() << " score is " << t << std::endl);
        score+=t;
      } else {
        double t=(*it)->evaluate(calc_derivs? &accum:NULL);
        IMP_LOG(TERSE, (*it)->get_name() << " score is " << t << std::endl);
        score+=t;
      }
    } else if (!(*it)->get_is_incremental()
               && incremental_restraints != INCREMENTAL) {
      double t=(*it)->evaluate(calc_derivs? &accum:NULL);
      IMP_LOG(TERSE, (*it)->get_name()<<  " score is " << t << std::endl);
      score+=t;
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

IMP_END_NAMESPACE
