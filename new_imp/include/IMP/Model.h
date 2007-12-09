/**
 *  \file Model.h   \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_MODEL_H
#define __IMP_MODEL_H

#include "IMP_config.h"
#include "ModelData.h"
#include "Restraint.h"
#include "RigidBody.h"
#include "State.h"
#include "boost/noncopyable.h"

namespace IMP
{

  class Particle;

//! Class for storing model, its restraints, constraints, and particles.
/** All attribute data for particles is stored through indexing in the
    model_data_ structure.
 */
class IMPDLLEXPORT Model: public boost::noncopyable
{

public:
  Model();
  ~Model();

  //! Get pointer to all model particle data.
  /** \return pointer to all model particle data.
   */
  ModelData* get_model_data() const;

  IMP_CONTAINER(Particle, particle, ParticleIndex);
 public:

  //! Add restraint set to the model.
  /** \param[in] restraint_set Pointer to the restraint set.
      \return the index of the newly-added restraint.
   */
  RestraintIndex add_restraint(Restraint* restraint_set);

  //! Get restraint set from the model.
  /** \param[in] i The RestraintIndex returned when adding.
      \exception std::out_of_range restraint index is out of range.
      \return pointer to the restraint.
   */
  Restraint* get_restraint(RestraintIndex i) const;

  //! Add state to the model.
  /** \param[in] state Pointer to the state.
      \return the index of the newly-added state.
   */
  StateIndex add_state(State* state);

  //! Get state from the model.
  /** \param[in] i The StateIndex returned when adding.
      \exception std::out_of_range state index is out of range.
      \return pointer to the state.
   */
  State* get_state(StateIndex i) const;

  //! Return the total number of restraints
  unsigned int number_of_restraints() const {
    return restraints_.size();
  }

  //! Evaluate all of the restraints in the model and return the score.
  /** \param[in] calc_derivs If true, also evaluate the first derivatives.
      \return The score.
   */
  Float evaluate(bool calc_derivs);

  //! Set up trajectory.
  /** \param[in] trajectory_path Path to file where the trajectory will be
                                 written.
      \param[in] trajectory_on True if trajectory is to be written as model
                               is optimized.
      \param[in] clear_file True if trajectory file should be cleared now.
  */
  void set_up_trajectory(const std::string trajectory_path = "trajectory.txt",
                         const bool trajectory_on = true,
                         const bool clear_file = true);

  //! Save the state of the model to the trajectory file.
  /** Currently hardcoded for "x", "y" and "z" particle float attributes.
   */
  void save_state();

  //! Show the model contents.
  /** \param[in] out Stream to write model description to.
   */
  void show (std::ostream& out = std::cout) const;

  std::string version() const {
    return "0.5.0";
  }
  std::string last_modified_by() const {
    return "Daniel Russel";
  }

protected:
  //! all of the data associated with the particles
  ModelData* model_data_;

  //! all base-level restraints and/or restraint sets of the model
  std::vector<Restraint*> restraints_;

  // all base-level restraints and/or restraint sets of the model
  std::vector<State*> states_;


  //! sets of particles that move as a single rigid body
  std::vector<RigidBody*> rigid_bodies_;

  //! trajectory file path
  std::string trajectory_path_;
  bool trajectory_on_;
  int frame_num_;
};





} // namespace IMP

#endif  /* __IMP_MODEL_H */
