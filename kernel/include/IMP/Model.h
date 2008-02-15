/**
 *  \file Model.h   \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_MODEL_H
#define __IMP_MODEL_H

#include "IMP_config.h"
#include "ScoreState.h"
#include "Object.h"
#include "base_types.h"

namespace IMP
{

class Particle;
class Restraint;
class ModelData;
typedef std::vector<Restraint*> Restraints;

//! Class for storing model, its restraints, constraints, and particles.
/** All attribute data for particles is stored through indexing in the
    model_data_ structure.
    \ingroup kernel
 */
class IMPDLLEXPORT Model: public Object
{
  friend class Restraint;
public:
  Model();
  ~Model();

  //! Get pointer to all model particle data.
  /** \return pointer to all model particle data.
   */
  ModelData* get_model_data() const;

  IMP_CONTAINER(Particle, particle, ParticleIndex);
  IMP_CONTAINER(ScoreState, score_state, ScoreStateIndex);
  IMP_CONTAINER(Restraint, restraint, RestraintIndex);
 public:


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
  std::auto_ptr<ModelData> model_data_;

  //! trajectory file path
  std::string trajectory_path_;
  bool trajectory_on_;
  int frame_num_;
};


} // namespace IMP

#endif  /* __IMP_MODEL_H */
