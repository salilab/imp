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
              {obj->set_model(this);},,
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

void Model::zero_derivatives() const {
  for (ParticleConstIterator pit = particles_begin();
       pit != particles_end(); ++pit) {
    (*pit)->zero_derivatives();
  }
}

double Model::do_evaluate(bool calc_derivs) const {
  // evaluate all of the active restraints to get score (and derivatives)
  // for current state of the model
  if (calc_derivs) {
    zero_derivatives();
  }
  double score = 0.0;
  DerivativeAccumulator accum;
  IMP_LOG(TERSE,
          "Begin evaluate restraints "
          << (calc_derivs?"with derivatives":"without derivatives")
          << std::endl);
  cur_stage_= EVALUATE;
  for (RestraintConstIterator it = restraints_begin();
       it != restraints_end(); ++it) {
    IMP_CHECK_OBJECT(*it);
    IMP_LOG(TERSE, "Evaluate restraint " << (*it)->get_name() << std::endl);
    double tscore = (*it)->evaluate((calc_derivs ? &accum : NULL));
    IMP_LOG(TERSE, "Restraint score is " << tscore << std::endl);
    score+= tscore;
  }
  IMP_LOG(TERSE, "End evaluate restraints." << std::endl);
  return score;
}


double Model::do_evaluate_incremental(bool calc_derivs) const {
  double score = 0.0;
  DerivativeAccumulator accum;
  cur_stage_= EVALUATE;
  IMP_LOG(TERSE,
          "Begin evaluate incremental restraints "
          << (calc_derivs?"with derivatives":"without derivatives")
          << std::endl);
  for (RestraintConstIterator it = restraints_begin();
       it != restraints_end(); ++it) {
    IMP_CHECK_OBJECT(*it);
    if ((*it)->get_is_incremental()) {
      IMP_LOG(TERSE, "Evaluate restraint " << (*it)->get_name() << std::endl);
      double tscore = (*it)->incremental_evaluate((calc_derivs ?
                                                   &accum : NULL));
      IMP_LOG(TERSE, "Restraint score is " << tscore << std::endl);
      score+= tscore;
    }
  }

  if (calc_derivs) {
    for (ParticleConstIterator pit = particles_begin();
         pit != particles_end(); ++pit) {
      (*pit)->old_->accumulate_derivatives_from(*pit, accum);
    }
  }
  IMP_LOG(TERSE, "End evaluate restraints." << std::endl);
  if (calc_derivs) {
    zero_derivatives();
  }
  IMP_LOG(TERSE,
          "Begin evaluate non-incremental restraints "
          << (calc_derivs?"with derivatives":"without derivatives")
          << std::endl);
  for (RestraintConstIterator it = restraints_begin();
       it != restraints_end(); ++it) {
    IMP_CHECK_OBJECT(*it);
    if (!(*it)->get_is_incremental()) {
      IMP_LOG(TERSE, "Evaluate restraint " << (*it)->get_name() << std::endl);
      double tscore = (*it)->evaluate((calc_derivs ? &accum : NULL));
      IMP_LOG(TERSE, "Restraint score is " << tscore << std::endl);
      score+= tscore;
    }
  }

  if (calc_derivs) {
    for (ParticleConstIterator pit = particles_begin();
         pit != particles_end(); ++pit) {
      (*pit)->accumulate_derivatives_from((*pit)->old_.get(), accum);
    }
  }
  IMP_LOG(TERSE, "End evaluate restraints." << std::endl);
  return score;
}


Float Model::evaluate(bool calc_derivs)
{
  IMP_OBJECT_LOG;
  IMP_CHECK_OBJECT(this);
  IMP_LOG(TERSE,
          "Begin Model::evaluate" << std::endl);

  before_evaluate();

  double score;
  if (get_is_incremental()) {
    score= do_evaluate_incremental(calc_derivs);
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
    for (ParticleIterator pit = particles_begin();
         pit != particles_end(); ++pit) {
      (*pit)->zero_derivatives();
    }
    for (RestraintIterator it= restraints_begin();
         it != restraints_end(); ++it) {
      if ((*it)->get_is_incremental()) {
        (*it)->evaluate(&da);
      }
    }
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
