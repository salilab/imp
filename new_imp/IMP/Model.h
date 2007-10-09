/*
 *  Model.h
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_MODEL_H
#define __IMP_MODEL_H

#include "IMP_config.h"
#include "Model_Data.h"
#include "Restraint_Set.h"
#include "Rigid_Body.h"

namespace imp
{

// Class for storing model, its restraints, constraints, and particles.
// All attribute data for particles is stored through indexing in the
// model_data_ structure.
class IMPDLLEXPORT Model
{
  friend class Particle_Iterator;
  friend class Restraint_Set_Iterator;

public:
  Model();
  ~Model();
  Model_Data* get_model_data(void);

  // particles
  size_t add_particle(Particle* particle);
  Particle* particle(size_t idx);

  // restraints
  void add_restraint_set(Restraint_Set* restraint_set);
  Restraint_Set* restraint_set(const std::string name);

  // for each restraint_set, call evaluate
  Float evaluate(bool calc_derivs);

  // set up trajectory file
  void set_up_trajectory(const std::string trajectory_path = "trajectory.txt",
                         const bool trajectory_on = true,
                         const bool clear_file = true);
  // set state to the trajectory file
  void save_state(void);

  void show (std::ostream& out = std::cout);
  std::string version(void) {
    return "0.5.0";
  }
  std::string last_modified_by(void) {
    return "Bret Peterson";
  }

protected:
  // all of the data associated with the particles
  Model_Data* model_data_;

  // particles themselves
  std::vector<Particle*> particles_;

  // all base-level restraints and/or restraint sets of the model
  std::vector<Restraint_Set*> restraint_sets_;

  // sets of particles that move as a single rigid body
  std::vector<Rigid_Body*> rigid_bodies_;

  // trajectory file path
  std::string trajectory_path_;
  bool trajectory_on_;
  int frame_num_;
};


// particle iterator
class IMPDLLEXPORT Particle_Iterator
{
public:
  Particle_Iterator() {}
  void reset(Model* model);
  bool next(void);
  Particle* get(void);

protected:
  int cur_;
  Model* model_;
};

// restraint set iterator
class IMPDLLEXPORT Restraint_Set_Iterator
{
public:
  Restraint_Set_Iterator() {}
  void reset(Model* model);
  bool next(void);
  Restraint_Set* get(void);

protected:
  int cur_;
  Model* model_;
};


} // namespace imp

#endif  /* __IMP_MODEL_H */
