/**
 *  \file Model.cpp \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include "IMP/Model.h"
#include "IMP/ModelData.h"
#include "IMP/Particle.h"
#include "IMP/log.h"
#include "IMP/Restraint.h"
#include "IMP/DerivativeAccumulator.h"
#include "mystdexcept.h"

namespace IMP
{

//! Constructor
Model::Model()
{
  model_data_ = new ModelData();
  frame_num_ = 0;
  trajectory_on_ = false;
}


//! Destructor
Model::~Model()
{
  IMP_CONTAINER_DELETE(Particle, particle);
  IMP_CONTAINER_DELETE(ScoreState, score_state);
  IMP_CONTAINER_DELETE(Restraint, restraint);
}


//! Get pointer to all model particle data.
/** \return pointer to all model particle data.
 */
ModelData* Model::get_model_data() const
{
  return model_data_;
}

IMP_CONTAINER_IMPL(Model, Restraint, restraint, RestraintIndex,
                     obj->set_model(this));


IMP_CONTAINER_IMPL(Model, Particle, particle, ParticleIndex,
                   obj->set_model(this, index));

IMP_CONTAINER_IMPL(Model, ScoreState, score_state, ScoreStateIndex,
                   obj->set_model(this));


//! Evaluate all of the restraints in the model and return the score.
/** \param[in] calc_derivs If true, also evaluate the first derivatives.
    \return The score.
 */
Float Model::evaluate(bool calc_derivs)
{
  // One or more particles may have been activated or deactivated.
  // Check each restraint to see if it changes its active status.
  IMP_LOG(VERBOSE,
          "Model evaluate (" << number_of_restraints() << " restraints):"
          << std::endl);

  if (model_data_->check_particles_active()) {
    IMP_LOG(VERBOSE,
          "Checking for active particles " << std::flush);

    for (RestraintIterator it = restraints_begin(); 
         it != restraints_end(); ++it) {
      (*it)->check_particles_active();
      IMP_LOG(VERBOSE, "." << std::flush);
    }

    model_data_->set_check_particles_active(false);
    IMP_LOG(VERBOSE, "done." << std::endl);
  }

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
          "Evaluating restraints " << calc_derivs << std::endl);
  for (RestraintIterator it = restraints_begin();
       it != restraints_end(); ++it) {
    IMP_CHECK_OBJECT(*it);
    IMP_LOG(VERBOSE, (*it)->get_name() << ": " << std::flush);
    Float tscore=0;
    if ((*it)->get_is_active()) {
      tscore = (*it)->evaluate(accpt);
    }
    IMP_LOG(VERBOSE, tscore << std::endl);
    score+= tscore;
  }
  IMP_LOG(VERBOSE, "done." << std::endl);

  // if trajectory is on and we are calculating derivatives, save state in
  // trajectory file
  // Arguably this should be done in the optimizer, but we would have to
  // find a way to support this in Modeller
  if (calc_derivs)
    save_state();

  IMP_LOG(VERBOSE, "Final score: " << score << std::endl);
  return score;
}


//! Set up trajectory.
/** \param[in] trajectory_path Path to file where the trajectory will be
                               written.
    \param[in] trajectory_on True if trajectory is to be written as model
                             is optimized.
    \param[in] clear_file True if trajectory file should be cleared now.
 */
void Model::set_up_trajectory(const std::string trajectory_path,
                              bool trajectory_on, bool clear_file)
{
  trajectory_path_ = trajectory_path;
  trajectory_on_ = trajectory_on;

  if (clear_file) {
    std::ofstream fout;
    fout.open(trajectory_path_.c_str(), std::ios_base::out);
    if (!fout.is_open()) {
      IMP_ERROR("Unable to initialize trajectory file: " << trajectory_path_
                << ". Trajectory writing is off.");
      trajectory_on_ = false;
    } else {
      frame_num_ = 0;
    }
  }
}

//! Save the state of the model to the trajectory file.
/** \warning Currently hardcoded for "x", "y" and "z" particle float attributes.
 */
void Model::save_state()
{
  if (!trajectory_on_) {
    return;
  }

  std::ofstream fout;

  FloatKey x("x"), y("y"), z("z");

  fout.open(trajectory_path_.c_str(), std::ios_base::app);
  if (!fout.is_open()) {
    IMP_ERROR("Unable to open trajectory file: "
              << trajectory_path_ << ". Trajectory is off. ");
    trajectory_on_ = false;
  } else {
    fout << "FRAME " << frame_num_ << std::endl;
    frame_num_++;
    fout << number_of_particles() << std::endl;

    // for each particle, output its current state
    FloatIndex fi;
    for (ParticleIterator it=particles_begin(); it != particles_end(); ++it) {
      fout << (*it)->get_value(x) << " ";

      fout << (*it)->get_value(y) << " ";

      fout << (*it)->get_value(z) << " ";
    }
    if (!fout) {
      IMP_ERROR("Error writing to trajectory file. Trajectory is off. ");
      trajectory_on_ = false;
    }
  }
}

//! Show the model contents.
/** \param[in] out Stream to write model description to.
 */
void Model::show(std::ostream& out) const
{
  out << std::endl << std::endl;
  out << "** Model **" << std::endl;

  out << "version: " << version() << "  ";
  out << "last_modified_by: " << last_modified_by() << std::endl;
  out << number_of_particles() << " particles" << std::endl;
  out << "Restraints:" << std::endl;
  for (RestraintConstIterator it = restraints_begin(); 
       it != restraints_end(); ++it) {
    IMP_CHECK_OBJECT(*it);
    out << (*it)->get_name() << std::endl;
  }

  internal::show_attributes(out);
}



}  // namespace IMP
