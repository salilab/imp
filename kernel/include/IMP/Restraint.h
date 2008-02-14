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
  IMP_LIST(Particle, particle, Particle*)

private:
  Model* model_;

  /* True if restraint has not been deactivated.
     If it is not active, evaluate should not be called
   */
  bool is_active_;
};

IMP_OUTPUT_OPERATOR(Restraint);

} // namespace IMP

#endif  /* __IMP_RESTRAINT_H */
