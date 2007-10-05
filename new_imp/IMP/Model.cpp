/*
 *  Model.cpp
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include "Model.h"
#include "Model_Data.h"
#include "Particle.h"
#include "log.h"

namespace imp
{

/**
  Constructor
 */

Model::Model ()
{
  model_data_ = new Model_Data();
  InitLog("log.txt");
  frame_num_ = 0;
}


/**
  Destructor
 */

Model::~Model ()
{
}


/**
 Get pointer to all model particle data.

  \return pointer to all model particle data.
 */

Model_Data* Model::get_model_data(void)
{
  return model_data_;
}

/**
  Get idx-th particle from the model. If idx is
  out of bounds, return null.

  \return idx-th particle in model.
 */

Particle* Model::particle(size_t idx)
{
  if ((idx < 0) || (idx >= particles_.size())) {
    return NULL;
  }

  return particles_[idx];
}


/**
  Add particle to the model.

  \param[in] particle Pointer to new particle.
 */

void Model::add_particle(Particle* particle)
{
  particle->set_model_data(model_data_);
  particles_.push_back(particle);
}


/**
 Add restraint set to model.

 \param[in] restraint_set Pointer to the restraint set.
*/

void Model::add_restraint_set(Restraint_Set* restraint_set)
{
  restraint_set->set_model_data(model_data_);
  restraint_sets_.push_back(restraint_set);
}


/**
 Get restraint set from model.

 \param[in] name Name of restraint within the model.
*/

Restraint_Set* Model::restraint_set(const std::string name)
{
  for (size_t i = 0; i < restraint_sets_.size(); i++) {
    if (restraint_sets_[i]->name() == name)
      return restraint_sets_[i];
  }

  return NULL;
}


/**
 Evaluate all of the restraints in the model and return the score.

 \param[in] out Stream to write model description to.
*/

Float Model::evaluate(bool calc_derivs)
{
  // One or more particles may have been activated or deactivated.
  // Check each restraint to see if it changes its active status.
  LogMsg(VERBOSE, "Model evaluate (" << restraint_sets_.size() << " restraint sets):");
  if (model_data_->check_particles_active()) {
    for (size_t i = 0; i < restraint_sets_.size(); i++) {
      restraint_sets_[i]->check_particles_active();
    }

    model_data_->set_check_particles_active(false);
  }

  // If calcualting derivatives, first set all derivatives to zero
  if (calc_derivs)
    model_data_->zero_derivatives();

  // evaluate all of the active restraints to get score (and derviatives)
  // for current state of the model
  Float score = 0.0;
  for (size_t i = 0; i < restraint_sets_.size(); i++) {
    LogMsg(VERBOSE, "Evaluating restraints " << restraint_sets_[i]->name() << ":");
    if (restraint_sets_[i]->is_active()) {
      score += restraint_sets_[i]->evaluate(calc_derivs);
    }

    LogMsg(VERBOSE, "Cumulative score: " << score);
  }

  // if trajectory is on and we are calculating derivatives, save state in trajectory file
  // Arguably this should be done in the optimizer, but we would have to find a way to
  // support this in Modeller
  if (calc_derivs)
    save_state();

  LogMsg(VERBOSE, "Final score: " << score);
  return score;
}


/**
 Set up trajectory.

 \param[in] trajectory_path Path to file where the trajectory will be written.
 \param[in] trajectory_on True if trajectory is to be written as model is optimized.
 \param[in] clear_file True if trajectory file should be cleared now.
*/

void Model::set_up_trajectory(const std::string trajectory_path, bool trajectory_on, bool clear_file)
{
  trajectory_path_ = trajectory_path;
  trajectory_on_ = trajectory_on;

  if (clear_file) {
    try {
      std::ofstream fout;
      fout.open(trajectory_path_.c_str(), std::ios_base::out);
      fout.close();

      frame_num_ = 0;
    }

    catch (...) {
      ErrorMsg("Unable to initialize trajectory file: " << trajectory_path_ << ". Trajectory is off. ");
      trajectory_on_ = false;
    }
  }
}

/**
 Save the state of the model to the trajectory file.
Currently hardcoded for "X", "Y" and "Z" particle float attributes.
*/

void Model::save_state(void)
{
  if (!trajectory_on_)
    return;

  try {
    std::ofstream fout;

    fout.open(trajectory_path_.c_str(), std::ios_base::app);

    fout << "FRAME " << frame_num_ << std::endl;
    frame_num_++;
    fout << particles_.size() << std::endl;

    // for each particle, output its current state
    Float_Index fi;
    for (size_t i = 0; i < particles_.size(); i++) {
      fi = particles_[i]->float_index("X");
      fout << model_data_->get_float(fi) << " ";

      fi = particles_[i]->float_index("Y");
      fout << model_data_->get_float(fi) << " ";

      fi = particles_[i]->float_index("Z");
      fout << model_data_->get_float(fi) << std::endl;
    }

    fout.close();
  }

  catch (...) {
    ErrorMsg("Unable to save to trajectory file: " << trajectory_path_ << ". Trajectory is off. ");
    trajectory_on_ = false;
  }
}

/**
 Show the model contents.

 \param[in] out Stream to write model description to.
*/

void Model::show(std::ostream& out)
{
  out << std::endl << std::endl;
  out << "** Model **" << std::endl;

  out << "version: " << version() << "  " << "last_modified_by: " << last_modified_by() << std::endl;
  out << particles_.size() << " particles" << std::endl;
  for (size_t i = 0; i < particles_.size(); i++) {
    particles_[i]->show(out);
  }

  for (size_t i = 0; i < restraint_sets_.size(); i++) {
    out  << std::endl << "* Restraint Set *" << std::endl;

    restraint_sets_[i]->show();
  }

}

// ####  Particle_Iterator ####
// Iterator returns all Particles in the Model


/**
  Reset the iterator so that after the next call to next(),
 get() will return the first particle.

 \param[in] model The model data that is being referenced.
 */

void Particle_Iterator::reset(Model* model)
{
  model_ = model;
  cur_ = -1;
}


/**
  Check if another particle is available, and if so,
 make sure it is called by the next call to get().

 \return True if another particle is available.
 */

bool Particle_Iterator::next(void)
{
  cur_ += 1;
  if (cur_ >= (int) model_->particles_.size()) {
    cur_ = (int) model_->particles_.size();
    return false;
  }

  return true;
}


/**
  Return the next available particle. Should only
  be called if next() returned True.

 \return True pointer to next particle (NULL if we are out of bounds).
 */

Particle* Particle_Iterator::get(void)
{
  if (cur_ >= (int) model_->particles_.size()) {
    return NULL;
  }

  return model_->particles_[cur_];
}


// ####  Restraint_Set_Iterator ####
// Iterator returns all restraint sets in the Model


/**
  Reset the iterator so that after the next call to next(),
 get() will return the first restraint_set.

 \param[in] model The model data that is being referenced.
 */

void Restraint_Set_Iterator::reset(Model* model)
{
  model_ = model;
  cur_ = -1;
}


/**
  Check if another restraint_set is available, and if so,
 make sure it is called by the next call to get().

 \return True if another restraint_set is available.
 */

bool Restraint_Set_Iterator::next(void)
{
  cur_ += 1;
  if (cur_ >= (int) model_->restraint_sets_.size()) {
    cur_ = (int) model_->restraint_sets_.size();
    return false;
  }

  return true;
}


/**
  Return the next available restraint_set. Should only
  be called if next() returned True.

 \return True pointer to next restraint_set (NULL if we are out of bounds).
 */

Restraint_Set* Restraint_Set_Iterator::get(void)
{
  if (cur_ >= (int) model_->restraint_sets_.size()) {
    return NULL;
  }

  return model_->restraint_sets_[cur_];
}




}  // namespace imp

