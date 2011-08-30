/**
 *  \file IMP/Restraint.h     \brief Abstract base class for all restraints.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_RESTRAINT_H
#define IMP_RESTRAINT_H

#include "kernel_config.h"
#include "DerivativeAccumulator.h"
#include "VersionInfo.h"
#include "Object.h"
#include "Pointer.h"
#include "log.h"
#include "utility.h"
#include "container_macros.h"
#include "container_base.h"
#include <vector>
#include <iostream>
#include <limits>

IMP_BEGIN_NAMESPACE


class Model;
class Restraint;
IMP_OBJECTS(Restraint,Restraints);


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

    \note Restraints will print a warning message if they are destroyed
    without ever having been added to a model as this is an easy mistake
    to make. To disable this warning for a particular restraint, call
    set_was_used(true).

    \implementation{Restraint, IMP_RESTRAINT, IMP::example::ExampleRestraint}
 */
class IMPEXPORT Restraint : public Object
{
public:
  Restraint(std::string name="Restraint %1%");

  //! Return the score for this restraint for the current state of the model.
  /** \return Current score.

      This method is equivalent to calling:
      \code
      model->evaluate(RestraintsTemp(1,this), calc_derivs)
      \endcode
   */
  double evaluate(bool calc_derivs) const;


  //! See Model::evaluate_if_good()
  double evaluate_if_good(bool calc_derivatives) const;

  /** This method is called to register a restrain with a model.
      Adding the restraint the model (Model::add_restraint()) will
      call this methodm but it can be called directly if one
      wants to use the restraint, just not as part of the normal
      scoring function.
   */
  virtual void set_model(Model* model);

  //! Return the model containing this restraint
  Model *get_model() const {
    IMP_INTERNAL_CHECK(model_,
               "get_model() called before set_model()");
    return model_;
  }

  //! Return true if this particle is part of a model
  bool get_is_part_of_model() const {
    return model_;
  }

#ifndef IMP_DOXYGEN
  virtual double unprotected_evaluate(DerivativeAccumulator *) const=0;
  virtual double unprotected_evaluate_if_good(DerivativeAccumulator *da,
                                              double /*max*/) const {
    return unprotected_evaluate(da);
  }
#endif

  /** \name Interactions
      Certain sorts of operations, such as evaluation of restraints in
      isolation, benefit from being able to determine which containers
      and particles are needed by which restraints.
      @{
  */
  virtual ContainersTemp get_input_containers() const=0;
  virtual ParticlesTemp get_input_particles() const=0;
  /** @} */

  //! Decompose this restraint into constituent terms
  /** Given the set of input particles, decompose the restraint into as
      simple parts as possible. For many restraints, the simplest
      part is simply the restraint itself.

      If a restraint can be decomposed, it should return a
      RestraintSet so that the maximum score and weight can be
      passed properly.

      The restraints returned have had set_model() called and so can
      be evaluated.
   */
  Restraint* create_decomposition() const;

  //! Decompose this restraint into constituent terms for the current conf
  /** Return a decomposition that is value for the current conformation,
      but will not necessarily be valid if any of the particles are
      changed. This is the same as create_decomposition() for
      non-conditional restraints.

      The restraints returned have had set_model() called and so can be
      evaluated.
   */
  Restraint* create_current_decomposition() const;

  void set_weight(Float weight);
  Float get_weight() const { return weight_; }
/** \name Filtering
      We are typically only interested in "good" conformations of
      the model. These are described by specifying maximum scores
      per restraint and for the whole model. Samplers, optimizers
      etc are free to ignore configurations they encounter which
      go outside these bounds.

      \note The maximum score for for the unweighted restraint
      (eg the score that is returned with weight 1).
      @{
  */
  double get_maximum_score() const {
    return max_;
  }
  void set_maximum_score(double s);
  /** @} */

  /** \brief For python, cast a generic Object to this type. Throw a
      ValueException of object is not the right type.*/
  static Restraint* get_from(Object *o) {
    return object_cast<Restraint>(o);
  }

  IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(Restraint);
 protected:
  /** A Restraint should override this if they want to decompose themselves
      for domino and other purposes. The returned restraints will be made
      in to a RestraintSet, if needed and the weight and maximum score
      set for the restraint set.
  */
  virtual Restraints do_create_decomposition() const {
    return Restraints(1, const_cast<Restraint*>(this));
  }
  /** A Restraint should override this if they want to decompose themselves
      for display and other purposes. The returned restraints will be made
      in to a RestraintSet, if needed and the weight and maximum score
      set for the restraint set.
   */
  virtual Restraints do_create_current_decomposition() const {
    return Restraints(1, const_cast<Restraint*>(this));
  }

private:
  /* This pointer should never be ref counted as Model has a
     pointer to this object.
   */
  WeakPointer<Model> model_;

  double weight_;
  double max_;
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
 public:
  // data cached by the model
  mutable Ints model_dependencies_;
  mutable double model_weight_;
#endif
};

IMP_END_NAMESPACE

#endif  /* IMP_RESTRAINT_H */
