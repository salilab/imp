/**
 *  \file Restraint.h     \brief Abstract base class for all restraints.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_RESTRAINT_H
#define __IMP_RESTRAINT_H

#include <vector>
#include <iostream>
#include <limits>

#include "IMP_config.h"
#include "Particle.h"
#include "DerivativeAccumulator.h"
#include "Object.h"
#include "utility.h"
#include "log.h"

namespace IMP
{

class Model;
/** \defgroup restraint General purpose restraints
    Classes to define and help in defining restraints. The restraints
    typically involve a Restraint which defines the set of tuples of Particles
    and then various functions which are applied to the tuples.
 */

/** \defgroup restraint General purpose restraints
    Classes to define and help in defining restraints. The restraints
    typically involve a Restraint which defines the set of tuples of Particles
    and then various functions which are applied to the tuples.
 */

//! Abstract class for representing restraints
class IMPDLLEXPORT Restraint : public Object
{
public:
  //! Initialize the Restraint
  Restraint();
  virtual ~Restraint();

  //! Return the score for this restraint for the current state of the model.
  /** \param[in] accum If not NULL, use this object to accumulate partial first
                       derivatives.
      \return Current score.
   */
  virtual Float evaluate(DerivativeAccumulator *accum) = 0;

  //! Set whether the restraint is active i.e. if it should be evaluated.
  /** \param[in] is_active If true, the restraint is active.
   */
  void set_is_active(const bool is_active);

  //! Get whether the restraint is active. i.e. if it should be evaluated.
  /** \return true if the restraint is active.
   */
  bool get_is_active() const;

  //! Check if all necessary particles are still active.
  /** If not, inactivate self. Called when at least one model particle
      has been inactivated.
   */
  virtual void check_particles_active();

  //! Show the current restraint.
  /** \param[in] out Stream to send restraint description to.
   */
  virtual void show(std::ostream& out=std::cout) const;

  virtual std::string version() const = 0;

  virtual std::string last_modified_by() const = 0;

  //! The model the restraint is part of.
  /** \param[in] model The model.
   */
  void set_model(Model* model);

  //! Return the model containing this restraint
  Model *get_model() const {
    IMP_assert(model_ != NULL,
               "get_model() called before set_model()");
    return model_;
  }

 protected:
  Particle *get_particle(unsigned int i) const;

  int add_particle(Particle *p) {
    IMP_CHECK_OBJECT(p);
    IMP_assert(p != NULL, "Can't add NULL particle");
    particles_.push_back(p);
    IMP_assert(particles_[0]->get_model() == particles_.back()->get_model(),
               "All particles in restraint must be from the same model.");
    IMP_assert(particles_.back()->get_model()
               ->get_particle(particles_.back()->get_index()) 
               == particles_.back(),
               "Model does not have pointer to particle.");
    IMP_assert(model_== NULL || model_==particles_.back()->get_model(),
               "Restraint model pointer and particle model pointer "
               << "don't match.");
    return particles_.size()-1;
  }

  //! Return the number of particles this restraint knows about
  unsigned int number_of_particles() const {
    return particles_.size();
  }

  //! Clear the internal list of particles
  void clear_particles() {
    return particles_.clear();
  }

  //! Add a bunch of particles together
  /** This is a very common operation and the bulk add will be faster
   */
  void add_particles(const Particles &ps) {
    particles_.insert(particles_.end(), ps.begin(), ps.end());
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
};

IMP_OUTPUT_OPERATOR(Restraint);


inline Particle *Restraint::get_particle(unsigned int i) const
{
  IMP_check(i < particles_.size(),
            "There are only " << particles_.size()
            << " but particle " << i << " was requested in restraint "
            << *this,
            IndexException("Not enough particles"));
  return particles_[i];
}

} // namespace IMP

#endif  /* __IMP_RESTRAINT_H */
