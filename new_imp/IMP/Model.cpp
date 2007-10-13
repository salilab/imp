/*
 *  Model.cpp
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include "Model.h"
#include "ModelData.h"
#include "Particle.h"
#include "log.h"

namespace IMP
{

/**
  Constructor
 */

Model::Model ()
{
  model_data_ = new ModelData();
  //InitLog("log.txt");
  frame_num_ = 0;
  trajectory_on_ = false;
}


/**
  Destructor
 */

Model::~Model ()
{
  IMP_LOG(VERBOSE,"Delete Model: beware of early Python calls to destructor.");
}


/**
 Get pointer to all model particle data.

  \return pointer to all model particle data.
 */

ModelData* Model::get_model_data(void) const
{
  return model_data_;
}

/**
  Get idx-th particle from the model. If idx is
  out of bounds, return null.

  \return idx-th particle in model.
 */

Particle* Model::get_particle(size_t idx) const
{
  if ((idx < 0) || (idx >= particles_.size())) {
    return NULL;
  }

  return particles_[idx];
}


/**
  Add particle to the model.

  \param[in] particle Pointer to new particle.
  \return index of particle within the model
 */

size_t Model::add_particle(Particle* particle)
{
  // a particle needs access to the model_data for
  // the model that it is a part of
  // particle now gets model_data from its constructor
  // particle->set_model_data(model_data_);

  // add the particle to the model list of particles
  particles_.push_back(particle);

  // return the particle index
  return particles_.size() - 1; 
}


/**
 Add restraint set to model.

 \param[in] restraint_set Pointer to the restraint set.
*/

void Model::add_restraint_set(RestraintSet* restraint_set)
{
  restraint_set->set_model_data(model_data_);
  restraint_sets_.push_back(restraint_set);
}


/**
 Get restraint set from model.

 \param[in] name Name of restraint within the model.
*/

RestraintSet* Model::restraint_set(const std::string name)
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
  IMP_LOG(VERBOSE, "Model evaluate (" << restraint_sets_.size() << " restraint sets):");
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
    IMP_LOG(VERBOSE, "Evaluating restraints " << restraint_sets_[i]->name() << ":");
    if (restraint_sets_[i]->is_active()) {
      score += restraint_sets_[i]->evaluate(calc_derivs);
    }

    IMP_LOG(VERBOSE, "Cumulative score: " << score);
  }

  // if trajectory is on and we are calculating derivatives, save state in trajectory file
  // Arguably this should be done in the optimizer, but we would have to find a way to
  // support this in Modeller
  if (calc_derivs)
    save_state();

  IMP_LOG(VERBOSE, "Final score: " << score);
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
    std::ofstream fout;
    fout.open(trajectory_path_.c_str(), std::ios_base::out);
    if (!fout.is_open()) {
      IMP_ERROR("Unable to initialize trajectory file: " << trajectory_path_
		<< ". Trajectory writing is off.");
      trajectory_on_=false;
    } else {
      frame_num_=0;
    }
  }
}

/**
 Save the state of the model to the trajectory file.
Currently hardcoded for "x", "y" and "z" particle float attributes.
*/

void Model::save_state(void)
{
  if (!trajectory_on_)
    return;

  //try {
    std::ofstream fout;

    fout.open(trajectory_path_.c_str(), std::ios_base::app);
    if (!fout.is_open()) {
      IMP_ERROR("Unable to open trajectory file: " 
		<< trajectory_path_ << ". Trajectory is off. ");
      trajectory_on_=false;
    } else {
      fout << "FRAME " << frame_num_ << std::endl;
      frame_num_++;
      fout << particles_.size() << std::endl;
      
      // for each particle, output its current state
      FloatIndex fi;
      for (size_t i = 0; i < particles_.size(); i++) {
	fi = particles_[i]->get_float_index("x");
	fout << model_data_->get_float(fi) << " ";
	
	fi = particles_[i]->get_float_index("y");
	fout << model_data_->get_float(fi) << " ";
	
	fi = particles_[i]->get_float_index("z");
	fout << model_data_->get_float(fi) << std::endl;
      }
      if (!fout) {
	IMP_ERROR("Error writing to trajectory file. Trajectory is off. ");
	trajectory_on_=false;
      }
    }
      //}

      //fout.close();
      //}

      /*catch (...) {
    ErrorMsg("Unable to save to trajectory file: " << trajectory_path_ << ". Trajectory is off. ");
    trajectory_on_ = false;
    }*/
}

/**
 Show the model contents.

 \param[in] out Stream to write model description to.
*/

void Model::show(std::ostream& out) const
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

// ####  ParticleIterator ####
// Iterator returns all Particles in the Model


/**
  Reset the iterator so that after the next call to next(),
 get() will return the first particle.

 \param[in] model The model data that is being referenced.
 */

void ParticleIterator::reset(Model* model)
{
  model_ = model;
  cur_ = -1;
}


/**
  Check if another particle is available, and if so,
 make sure it is called by the next call to get().

 \return True if another particle is available.
 */

bool ParticleIterator::next(void)
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

Particle* ParticleIterator::get(void)
{
  if (cur_ >= (int) model_->particles_.size()) {
    return NULL;
  }

  return model_->particles_[cur_];
}


// ####  RestraintSetIterator ####
// Iterator returns all restraint sets in the Model


/**
  Reset the iterator so that after the next call to next(),
 get() will return the first restraint_set.

 \param[in] model The model data that is being referenced.
 */

void RestraintSetIterator::reset(Model* model)
{
  model_ = model;
  cur_ = -1;
}


/**
  Check if another restraint_set is available, and if so,
 make sure it is called by the next call to get().

 \return True if another restraint_set is available.
 */

bool RestraintSetIterator::next(void)
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

RestraintSet* RestraintSetIterator::get(void)
{
  if (cur_ >= (int) model_->restraint_sets_.size()) {
    return NULL;
  }

  return model_->restraint_sets_[cur_];
}




}  // namespace IMP

