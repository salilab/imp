/**
 *  \file Model.cpp \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/Model.h"
#include "IMP/ModelData.h"
#include "IMP/Particle.h"
#include "IMP/log.h"
#include "IMP/Restraint.h"
#include "IMP/DerivativeAccumulator.h"
#include "IMP/ScoreState.h"

namespace IMP
{

//! Constructor
Model::Model(): model_data_(new ModelData())
{
}


//! Destructor
Model::~Model()
{
}

IMP_CONTAINER_IMPL(Model, Restraint, restraint, RestraintIndex,
                     obj->set_model(this));

IMP_CONTAINER_IMPL(Model, Particle, particle, ParticleIndex,
                   {obj->set_model(this, index);});

IMP_CONTAINER_IMPL(Model, ScoreState, score_state, ScoreStateIndex,
                   {obj->set_model(this);});


Float Model::evaluate(bool calc_derivs)
{
  IMP_LOG(VERBOSE,
          "Model evaluate (" << number_of_restraints() << " restraints):"
          << std::endl);
  // If calcualting derivatives, first set all derivatives to zero
  if (calc_derivs)
    model_data_->zero_derivatives();

  IMP_LOG(VERBOSE,
          "Updating ScoreStates " << std::flush);
  for (ScoreStateIterator it = score_states_begin(); it != score_states_end();
       ++it) {
    IMP_CHECK_OBJECT(*it);
    (*it)->update();
    IMP_LOG(VERBOSE, "." << std::flush);
  }
  IMP_LOG(VERBOSE, "done." << std::endl);

  // evaluate all of the active restraints to get score (and derivatives)
  // for current state of the model
  Float score = 0.0;
  DerivativeAccumulator accum;
  DerivativeAccumulator *accpt = (calc_derivs ? &accum : NULL);

  IMP_LOG(VERBOSE,
          "Evaluating restraints " 
          << (calc_derivs?"with derivatives":"without derivatives")
              << std::endl);
  for (RestraintIterator it = restraints_begin();
       it != restraints_end(); ++it) {
    IMP_CHECK_OBJECT(*it);
    IMP_LOG(VERBOSE, **it);
    Float tscore=0;
    if ((*it)->get_is_active()) {
      tscore = (*it)->evaluate(accpt);
    }
    IMP_LOG(VERBOSE, "Restraint score is " << tscore << std::endl);
    score+= tscore;
  }
  IMP_LOG(VERBOSE, "done." << std::endl);

  IMP_LOG(VERBOSE, "Final score: " << score << std::endl);
  return score;
}

void Model::show(std::ostream& out) const
{
  out << std::endl << std::endl;
  out << "** Model **" << std::endl;

  get_version_info().show(out);

  out << number_of_particles() << " particles" << std::endl;

  internal::show_attributes(out);
}



}  // namespace IMP
