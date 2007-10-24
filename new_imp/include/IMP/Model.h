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
#include "restraints/Restraint.h"
#include "RigidBody.h"
#include "State.h"
#include "boost/noncopyable.h"

namespace IMP
{

//! Class for storing model, its restraints, constraints, and particles.
/** All attribute data for particles is stored through indexing in the
    model_data_ structure.
 */
class IMPDLLEXPORT Model: public boost::noncopyable
{
  friend class ParticleIterator;
  friend class RestraintIterator;

public:
  Model();
  ~Model();

  //! Get pointer to all model particle data.
  /** \return pointer to all model particle data.
   */
  ModelData* get_model_data(void) const;

  //! Add a particle to the model.
  /** \param[in] particle Pointer to new particle.
      \return index of particle within the model
   */
  ParticleIndex add_particle(Particle* particle);

  //! Get a pointer to a particle in the model, or null if out of bounds.
  /** \param[in] idx  Index of particle
      \return Pointer to the particle, or null if out of bounds.
   */
  Particle* get_particle(ParticleIndex idx) const;

  //! Get the total number of particles in the model.
  unsigned int number_of_particles() const {
    return particles_.size();
  }

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
  void save_state(void);

  //! Show the model contents.
  /** \param[in] out Stream to write model description to.
   */
  void show (std::ostream& out = std::cout) const;

  std::string version(void) const {
    return "0.5.0";
  }
  std::string last_modified_by(void) const {
    return "Bret Peterson";
  }

protected:
  //! all of the data associated with the particles
  ModelData* model_data_;

  //! particles themselves
  std::vector<Particle*> particles_;

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


//! Particle iterator; returns all Particles in the Model.
class IMPDLLEXPORT ParticleIterator
{
public:
  ParticleIterator() {}

  //! Reset the iterator.
  /** After the next call to next(), get() will return the first particle.
      \param[in] model  The model that is being referenced.
   */
  void reset(Model* model);

  //! Move to the next particle.
  /** Check if another particle is available, and if so, make sure it is
      called by the next call to get().
      \return True if another particle is available.
   */
  bool next(void);

  //! Return the next particle.
  /** Should only be called if next() returned True.
      \return Pointer to the next particle, or null if out of bounds.
   */
  Particle* get(void);

protected:
  int cur_;
  Model* model_;
};

//! Restraint set iterator
/** Returns all restraint sets in the Model.
 */
class IMPDLLEXPORT RestraintIterator
{
public:
  RestraintIterator() {}

  //! Reset the iterator.
  /** After the next call to next(), get() will return the first restraint set.
      \param[in] model The model that is being referenced.
   */
  void reset(Model* model);

  //! Move to the next restraint set if available.
  /** Check if another restraint set is available, and if so,
      make sure it is called by the next call to get().
      \return True if another restraint set is available.
   */
  bool next(void);

  //! Return the current restraint set.
  /** Should only be called if next() returned True.
      \return pointer to the restraint set, or null if out of bounds.
   */
  Restraint* get(void);

protected:
  int cur_;
  Model* model_;
};


inline std::ostream &operator<<(std::ostream &out, const Model &s)
{
  s.show(out);
  return out;
}


} // namespace IMP

#endif  /* __IMP_MODEL_H */
