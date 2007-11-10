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
#include "IMP/DerivativeAccumulator.h"
#include "mystdexcept.h"

namespace IMP
{

//! Constructor
Model::Model ()
{
  model_data_ = new ModelData();
  frame_num_ = 0;
  trajectory_on_ = false;
}


//! Destructor
Model::~Model ()
{
  IMP_LOG(VERBOSE,"Delete Model: beware of early Python calls to destructor.");
}


//! Get pointer to all model particle data.
/** \return pointer to all model particle data.
 */
ModelData* Model::get_model_data() const
{
  return model_data_;
}

//! Add a particle to the model.
/** \param[in] particle Pointer to new particle.
    \return index of particle within the model
 */
ParticleIndex Model::add_particle(Particle* particle)
{
  // a particle needs access to the model_data for
  // the model that it is a part of
  // particle now gets model_data from its constructor
  // particle->set_model_data(model_data_);

  // add the particle to the model list of particles
  particles_.push_back(particle);
  ParticleIndex pi(particles_.size() - 1);
  particle->set_model(this, pi);

  // return the particle index
  return pi; 
}



//! Get a pointer to a particle in the model, or null if out of bounds.
/** \param[in] idx  Index of particle
    \return Pointer to the particle, or null if out of bounds.
 */
Particle* Model::get_particle(ParticleIndex idx) const
{
  if ((idx < 0) || (idx >= particles_.size())) {
    return NULL;
  }

  return particles_[idx.get_index()];
}


//! Add restraint set to the model.
/** \param[in] restraint_set Pointer to the restraint set.
    \return the index of the newly-added restraint.
 */
RestraintIndex Model::add_restraint(Restraint* restraint_set)
{
  restraints_.push_back(restraint_set);
  RestraintIndex ri(restraints_.size()-1);
  restraint_set->set_model(this);
  return ri;
}


//! Get state from the model.
/** \param[in] i The StateIndex returned when adding.
    \exception std::out_of_range state index is out of range.
    \return pointer to the state.
 */
State* Model::get_state(StateIndex i) const
{
  IMP_check(i.get_index() < states_.size(), 
            "Out of range State requested",
            std::out_of_range("Invalid State requested"));
  return states_[i.get_index()];
}


//! Add state to the model.
/** \param[in] state Pointer to the state.
    \return the index of the newly-added state.
*/
StateIndex Model::add_state(State* state)
{
  state->set_model_data(model_data_);
  states_.push_back(state);
  return states_.size()-1;
}


//! Get restraint set from the model.
/** \param[in] i The RestraintIndex returned when adding.
    \exception std::out_of_range restraint index is out of range.
    \return pointer to the restraint.
 */
Restraint* Model::get_restraint(RestraintIndex i) const
{
  IMP_check(i.get_index() < restraints_.size(), 
            "Out of range restraint requested",
            std::out_of_range("Invalid restraint requested"));
  return restraints_[i.get_index()];
}

//! Evaluate all of the restraints in the model and return the score.
/** \param[in] calc_derivs If true, also evaluate the first derivatives.
    \return The score.
 */
Float Model::evaluate(bool calc_derivs)
{
  // One or more particles may have been activated or deactivated.
  // Check each restraint to see if it changes its active status.
  IMP_LOG(VERBOSE, "Model evaluate (" << restraints_.size()
                   << " restraint sets):");
  if (model_data_->check_particles_active()) {
    for (size_t i = 0; i < restraints_.size(); i++) {
      restraints_[i]->check_particles_active();
    }

    model_data_->set_check_particles_active(false);
  }

  // If calcualting derivatives, first set all derivatives to zero
  if (calc_derivs)
    model_data_->zero_derivatives();

  for (size_t i = 0; i < states_.size(); i++) {
    IMP_LOG(VERBOSE, "Updating state " << states_[i]->get_name() << ":");
    states_[i]->update();
  }

  // evaluate all of the active restraints to get score (and derivatives)
  // for current state of the model
  Float score = 0.0;
  DerivativeAccumulator accum;
  DerivativeAccumulator *accpt = (calc_derivs ? &accum : NULL);
  for (size_t i = 0; i < restraints_.size(); i++) {
    IMP_LOG(VERBOSE, "Evaluating restraint " << restraints_[i]->get_name()
                     << ":");
    if (restraints_[i]->get_is_active()) {
      score += restraints_[i]->evaluate(accpt);
    }

    IMP_LOG(VERBOSE, "Cumulative score: " << score);
  }

  // if trajectory is on and we are calculating derivatives, save state in
  // trajectory file
  // Arguably this should be done in the optimizer, but we would have to
  // find a way to support this in Modeller
  if (calc_derivs)
    save_state();

  IMP_LOG(VERBOSE, "Final score: " << score);
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
      trajectory_on_=false;
    } else {
      frame_num_=0;
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
    trajectory_on_=false;
  } else {
    fout << "FRAME " << frame_num_ << std::endl;
    frame_num_++;
    fout << particles_.size() << std::endl;

    // for each particle, output its current state
    FloatIndex fi;
    for (size_t i = 0; i < particles_.size(); i++) {
      fout << particles_[i]->get_value(x) << " ";

      fout <<  particles_[i]->get_value(y) << " ";

      fout << particles_[i]->get_value(z) << " ";
    }
    if (!fout) {
      IMP_ERROR("Error writing to trajectory file. Trajectory is off. ");
      trajectory_on_=false;
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

  out << "version: " << version() << "  " << "last_modified_by: "
      << last_modified_by() << std::endl;
  out << particles_.size() << " particles" << std::endl;
  for (size_t i = 0; i < particles_.size(); i++) {
    particles_[i]->show(out);
  }

  for (size_t i = 0; i < restraints_.size(); i++) {
    out  << std::endl << "* Restraint *" << std::endl;

    restraints_[i]->show(out);
  }

}

//! Reset the iterator.
/** After the next call to next(), get() will return the first particle.
    \param[in] model  The model that is being referenced.
 */
void ParticleIterator::reset(Model* model)
{
  model_ = model;
  cur_ = -1;
}


//! Move to the next particle.
/** Check if another particle is available, and if so, make sure it is
    called by the next call to get().
    \return True if another particle is available.
 */
bool ParticleIterator::next()
{
  cur_ += 1;
  if (cur_ >= (int) model_->particles_.size()) {
    cur_ = (int) model_->particles_.size();
    return false;
  }

  return true;
}


//! Return the next particle.
/** Should only be called if next() returned True.
    \return Pointer to the next particle, or null if out of bounds.
 */
Particle* ParticleIterator::get()
{
  if (cur_ >= (int) model_->particles_.size()) {
    return NULL;
  }

  return model_->particles_[cur_];
}


//! Reset the iterator.
/** After the next call to next(), get() will return the first restraint set.
    \param[in] model The model that is being referenced.
 */
void RestraintIterator::reset(Model* model)
{
  model_ = model;
  cur_ = -1;
}


//! Move to the next restraint set if available.
/** Check if another restraint set is available, and if so,
    make sure it is called by the next call to get().
    \return True if another restraint set is available.
 */
bool RestraintIterator::next()
{
  cur_ += 1;
  if (cur_ >= (int) model_->restraints_.size()) {
    cur_ = (int) model_->restraints_.size();
    return false;
  }

  return true;
}


//! Return the current restraint set.
/** Should only be called if next() returned True.
    \return pointer to the restraint set, or null if out of bounds.
 */ 
Restraint* RestraintIterator::get()
{
  if (cur_ >= (int) model_->restraints_.size()) {
    return NULL;
  }

  return model_->restraints_[cur_];
}


}  // namespace IMP
