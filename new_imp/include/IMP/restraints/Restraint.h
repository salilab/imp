/**
 *  \file Restraint.h     \brief Abstract base class for all restraints.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_RESTRAINT_H
#define __IMP_RESTRAINT_H

#include <vector>
#include <iostream>

#include "../IMP_config.h"
#include "../ModelData.h"
#include "../ScoreFunc.h"
#include "../DerivativeAccumulator.h"
#include "../boost/noncopyable.h"
#include "../utility.h"
#include "../log.h"

namespace IMP
{

class Model;

//! Abstract class for representing restraints
class IMPDLLEXPORT Restraint : public boost::noncopyable
{

public:
  Restraint(std::string name=std::string());
  virtual ~Restraint();

  //! Return the score for this restraint for the current state of the model.
  /** \param[in] accum If not NULL, use this object to accumulate partial first
                       derivatives.
      \return Current score.
   */
  virtual Float evaluate(DerivativeAccumulator *accum) {return 0;}

  //! Set whether the restraint is active i.e. if it should be evaluated.
  /** \param[in] is_active If true, the restraint is active.
   */
  void set_is_active(const bool is_active);

  //! Get whether the restraint is active. i.e. if it should be evaluated.
  /** \return true if the restraint is active.
   */
  bool get_is_active(void) const;

  //! Check if all necessary particles are still active.
  /** If not, inactivate self. Called when at least one model particle
      has been inactivated.
   */
  virtual void check_particles_active(void);

  //! Show the current restraint.
  /** \param[in] out Stream to send restraint description to.
   */
  virtual void show(std::ostream& out) const;

  virtual std::string version(void) const {
    return std::string("unknown");
  }

  virtual std::string last_modified_by(void) const {
    return std::string("unknown");
  }

  //! Get the name of the restraint
  const std::string& get_name() const {
    return name_;
  }

  //! Set the name of the restraint
  void set_name(const std::string &name) {
    name_=name;
  }

  Particle *get_particle(unsigned int i) const;

  int add_particle(Particle *p) {
    particles_.push_back(p);
    return particles_.size()-1;
  }

  //! The model the restraint is part of.
  /** \param[in] model The model.
   */
  void set_model(Model* model){
    model_=model;
  }



  //! Return the model containing this restraint
  Model *get_model() const {
    return model_;
  }

  //! Return the number of particles this restraint knows about
  unsigned int number_of_particles() const {
    return particles_.size();
  }

private:
  //! all of the particle data
  Model* model_;

  /** restraint is active if active_ AND particles_active_
      true if restraint has not been deactivated
      if it is not active, evaluate should not be called
   */
  bool is_active_;

  //! true if all particles that restraint uses are active
  bool are_particles_active_;

  std::vector<Particle*> particles_;

  std::string name_;
};

IMP_OUTPUT_OPERATOR(Restraint);


inline Particle *Restraint::get_particle(unsigned int i) const {
    IMP_check(i < particles_.size(),
              "There are only " << particles_.size()
              << " but particle " << i << " was requested in restraint "
              << *this,
              IndexException("Not enough particles"));
    return particles_[i];
  }

} // namespace IMP

#endif  /* __IMP_RESTRAINT_H */
