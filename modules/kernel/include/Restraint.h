/**
 *  \file IMP/Restraint.h
 *  \brief Abstract base class for all restraints.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_RESTRAINT_H
#define IMPKERNEL_RESTRAINT_H

#include <IMP/kernel_config.h>
#include "ModelObject.h"
#include "ScoreAccumulator.h"
#include "DerivativeAccumulator.h"
#include "constants.h"
#include "base_types.h"
#include <IMP/InputAdaptor.h>
#include <IMP/deprecation_macros.h>
#include <IMP/nullptr.h>
#include <IMP/RestraintInfo.h>

IMPKERNEL_BEGIN_NAMESPACE
class DerivativeAccumulator;

//! A restraint is a term in an \imp ScoringFunction.
/**
    To implement a new restraint, just implement the two methods:
    - IMP::Restraint::do_add_score_and_derivatives()
    - IMP::ModelObject::do_get_inputs();
    and use the macro to handle IMP::Object
    - IMP_OBJECT_METHODS()

    \note When logging is VERBOSE, restraints should print enough information
    in evaluate to reproduce the entire flow of data in evaluate. When
    logging is TERSE the restraint should print out only a constant number of
    lines per evaluate call.

    \note Physical restraints should use the units of kcal/mol for restraint
    values and kcal/mol/A for derivatives.

    When implementing an expensive restraint it makes sense to support early
    abort of evaluation if the user is only interested in good scores or scores
    below a threshold. To do this, look at the fields of the ScoreAccumulator
    object such as
    - ScoreAccumulator::get_is_evaluate_if_below(),
    - ScoreAccumulator::get_is_evaluate_if_good()
    - ScoreAccumulator::get_maximum()

    \headerfile Restraint.h "IMP/Restraint.h"

    See IMP::example::ExampleRestraint for an example.
 */
class IMPKERNELEXPORT Restraint : public ModelObject {
 public:
  /** Create a restraint and register it with the model. The restraint is
      not added to the implicit scoring function in the Model.*/
  Restraint(Model *m, std::string name);

  /** Compute and return the current score for the restraint.
   */
  double get_score() const;

#ifndef IMP_DOXYGEN
  //! Return the score for this restraint for the current state of the model.
  /** \return Current score.
   */
  double evaluate(bool calc_derivs) const;

  double evaluate_if_good(bool calc_derivatives) const;

  //! \see Model::evaluate_with_maximum()
  double evaluate_if_below(bool calc_derivatives, double max) const;

  /** \name Evaluation implementation
      These methods are called in order to perform the actual restraint
      scoring. The restraints should assume that all appropriate ScoreState
      objects have been updated and so that the input particles and containers
      are up to date. The returned score should be the unweighted score.

      \note These functions probably should be called \c do_evaluate, but
      were grandfathered in.
      \note Although the returned score is unweighted, the DerivativeAccumulator
      passed in should be properly weighted.
      @{
  */
  //! Return the unweighted score for the restraint.
  virtual double unprotected_evaluate(DerivativeAccumulator *da) const;
  /** The function calling this will treat any score >= get_maximum_score
      as bad and so can return early as soon as such a situation is found.*/
  virtual double unprotected_evaluate_if_good(DerivativeAccumulator *da,
                                              double max) const {
    IMP_UNUSED(max);
    return unprotected_evaluate(da);
  }

  //! The function calling this will treat any score >= max as bad.
  virtual double unprotected_evaluate_if_below(DerivativeAccumulator *da,
                                               double max) const {
    IMP_UNUSED(max);
    return unprotected_evaluate(da);
  }
/** @} */

#endif

  //! \return static key:value information about this restraint, or null.
  /** \return a set of key:value pairs that contain static information
      about this restraint (i.e. information that doesn't change during
      a sampling run, such as the type of restraint or filename from
      which information is read). Usually this includes a "type" key with
      the fully qualified classname (e.g. IMP.mymodule.MyRestraint).
      If no such information is available, a null pointer is returned.
      This information is used when writing restraints to files, e.g. by
      the IMP.rmf module.
    */
  virtual RestraintInfo *get_static_info() const {
    return nullptr;
  }

  //! \return dynamic key:value information about this restraint, or null.
  /** \return a set of key:value pairs that contain dynamic information
      about this restraint (i.e. information that changes during a sampling
      run, such as scores or cross correlations).
      If no such information is available, a null pointer is returned.
      This information is used when writing restraints to files, e.g. by
      the IMP.rmf module.
    */
  virtual RestraintInfo *get_dynamic_info() const {
    return nullptr;
  }

  //! Perform the actual restraint scoring.
  /** The restraints should assume that all appropriate ScoreState
      objects have been updated and so that the input particles and containers
      are up to date. The returned score should be the unweighted score.
  */
  void add_score_and_derivatives(ScoreAccumulator sa) const;

  //! Decompose this restraint into constituent terms
  /** Given the set of input particles, decompose the restraint into parts
      that are as simple as possible. For many restraints, the simplest
      part is simply the restraint itself.

      If a restraint can be decomposed, it should return a
      RestraintSet so that the maximum score and weight can be
      passed properly.

      The restraints returned have had set_model() called and so can
      be evaluated.
   */
  Restraint *create_decomposition() const;

  //! Decompose this restraint into constituent terms for the current conf
  /** \return a decomposition that is value for the current conformation,
      but will not necessarily be valid if any of the particles are
      changed. This is the same as create_decomposition() for
      non-conditional restraints.

      The restraints returned have had set_model() called and so can be
      evaluated.
   */
  Restraint *create_current_decomposition() const;

  /** \name Weights
      Each restraint's contribution to the model score is weighted. The
      total weight for the restraint is the some over all the paths containing
      it. That is, if a restraint is in a RestraintSet with weight .5 and
      another with weight 2, and the restraint itself has weight 3, then the
      total weight of the restraint is \f$.5 \cdot 3 + 2 \cdot 3 = 7.5  \f$.
      @{
  */
  void set_weight(Float weight);
  Float get_weight() const { return weight_; }
  /** @} */
  /** \name Filtering
      We are typically only interested in "good" conformations of
      the model. These are described by specifying maximum scores
      per restraint (or RestraintSet). Samplers, optimizers
      etc are free to ignore configurations they encounter which
      go outside these bounds.

      \note The maximum score is for the unweighted restraint.
       That is, the restraint evaluation is bad if the value
       is greater than the maximum score divided by the weight.
      @{
  */
  double get_maximum_score() const { return max_; }
  void set_maximum_score(double s);
/** @} */

//! Create a scoring function with only this restraint.
/** \note This method cannot be implemented in Python due to memory
      management issues (and the question of why you would ever
      want to).
   */
#ifndef SWIG
  virtual
#endif
      ScoringFunction *
          create_scoring_function(double weight = 1.0,
                                  double max = NO_MAX) const;
#if !defined(IMP_DOXYGEN)
  void set_last_score(double s) const { last_score_ = s; }
#endif

  /** Return the (unweighted) score for this restraint last time it was
      evaluated.
      \note If some sort of special evaluation (eg Model::evaluate_if_good())
      was the last call, the score, if larger than the max, is not accurate.
   */
  virtual double get_last_score() const { return last_score_; }
  /** Return whether this restraint violated its maximum last time it was
      evaluated.
   */
  bool get_was_good() const { return get_last_score() < max_; }

  IMP_REF_COUNTED_DESTRUCTOR(Restraint);

 protected:
  /** A Restraint should override this if it wants to decompose itself
      for domino and other purposes. The returned restraints will be made
      into a RestraintSet if needed, with suitable weight and maximum score.
  */
  virtual Restraints do_create_decomposition() const {
    return Restraints(1, const_cast<Restraint *>(this));
  }
  /** A Restraint should override this if it wants to decompose itself
      for display and other purposes. The returned restraints will be made
      into a RestraintSet if needed, with suitable weight and maximum score.

      The returned restraints should be only the non-zero terms and should
      have their last scores set appropriately.
   */
  virtual Restraints do_create_current_decomposition() const {
    return do_create_decomposition();
  }
  //! A restraint should override this to compute the score and derivatives.
  virtual void do_add_score_and_derivatives(ScoreAccumulator sa) const;

  /** No outputs. */
  ModelObjectsTemp do_get_outputs() const { return ModelObjectsTemp(); }

 private:
  ScoringFunction *create_internal_scoring_function() const;

  double weight_;
  double max_;
  mutable double last_score_;
  // cannot be released outside the class
  mutable Pointer<ScoringFunction> cached_internal_scoring_function_;
};

//! Provide a consistent interface for things that take Restraints as arguments.
/**
    \note Passing an empty list of restraints should be supported, but problems
    could arise, so be alert (the problems would not be subtle).
*/
class IMPKERNELEXPORT RestraintsAdaptor :
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
    public Restraints
#else
    public InputAdaptor
#endif
    {
  static Restraint *get(Restraint *r) { return r; }

 public:
  RestraintsAdaptor() {}
  RestraintsAdaptor(const Restraints &sf) : Restraints(sf) {}
  RestraintsAdaptor(const RestraintsTemp &sf)
      : Restraints(sf.begin(), sf.end()) {}
  RestraintsAdaptor(Restraint *sf) : Restraints(1, sf) {}
#ifndef IMP_DOXYGEN
  template <class T>
  RestraintsAdaptor(internal::PointerBase<T> t)
      : Restraints(1, get(t)) {}
#endif
};

//! Return the decomposition of a list of restraints.
IMPKERNELEXPORT Restraints create_decomposition(const RestraintsTemp &rs);

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_RESTRAINT_H */
