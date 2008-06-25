/**
 *  \file Restraint.h     \brief Abstract base class for all restraints.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_RESTRAINT_H
#define __IMP_RESTRAINT_H

#include "IMP_config.h"
#include "DerivativeAccumulator.h"
#include "Model.h"
#include "Particle.h"
#include "VersionInfo.h"
#include "internal/Object.h"
#include "internal/ObjectPointer.h"
#include "log.h"
#include "utility.h"

#include <vector>
#include <iostream>
#include <limits>



namespace IMP
{

class Model;
/** \defgroup restraint General purpose restraints
    Classes to define and help in defining restraints. The restraints
    typically involve a Restraint which defines the set of tuples of Particles
    and then various functions which are applied to the tuples.
 */

//! Abstract class for representing restraints
/** Restraints should take their score function or UnaryFunction
    as the first argument. Restraints which act on large numbers of
    particles should allow the particle list to be skipped in the
    constructor and should provide methods so that the set of particles
    can be modified after construction.

    A restraint can be added to the model multiple times or to multiple 
    restraint sets in the same model.

    \note When logging is VERBOSE, restraints should print enough information
    in evaluate to reproduce the the entire flow of data in evaluate. When
    logging is TERSE the restraint should print out only a constant number of
    lines per evaluate call.

    \note Physical restraints should use the units of kcal/mol for restraint
    values and kcal/mol/A for derivatives.
 */
class IMPDLLEXPORT Restraint : public internal::RefCountedObject
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
      The output of show may take several lines and should end in a newline.
   */
  virtual void show(std::ostream& out=std::cout) const;

  //! \return version and authorship information.
  virtual VersionInfo get_version_info() const = 0;

  //! The model the restraint is part of.
  /** \param[in] model The model.
   */
  void set_model(Model* model);

  //! Return the model containing this restraint
  Model *get_model() const {
    IMP_assert(model_,
               "get_model() called before set_model()");
    return model_.get();
  }

  IMP_LIST(protected, Particle, particle, Particle*)

private:
  internal::ObjectPointer<Model, false> model_;

  /* True if restraint has not been deactivated.
     If it is not active, evaluate should not be called
   */
  bool is_active_;
};

IMP_OUTPUT_OPERATOR(Restraint);

} // namespace IMP

#endif  /* __IMP_RESTRAINT_H */
