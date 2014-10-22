/**
 *  \file IMP/kernel/ScoringFunction.h
 *  \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_SCORING_FUNCTION_H
#define IMPKERNEL_SCORING_FUNCTION_H

#include <IMP/kernel/kernel_config.h>
#include "base_types.h"
#include "dependency_graph.h"
#include "Restraint.h"
#include "ModelObject.h"
#include <IMP/base/InputAdaptor.h>
#include <IMP/base/Pointer.h>

#include <limits>

IMPKERNEL_BEGIN_NAMESPACE
class Model;

//! Represents a scoring function on the model.
/** The Model has a default scoring function
    (kernel::Model::get_model_scoring_function()), but it can be useful to use
    others in different contexts during a sampling process.

A call to the evaluate() method prompts the following events:
1. determine set of ScoreState objects needed by the Restraint objects
being evaluated (this is cached)
1. call ScoreState::before_evaluate() on each of them to update
    configuration
1. call Restraint::unprotected_evaluate() to compute scores
    [and add derivatives in the particles]
1. [call ScoreState::after_evaluate() on each score state to update derivatives]
1. score returned

    \headerfile ScoringFunction.h "IMP/ScoringFunction.h"

*/
class IMPKERNELEXPORT ScoringFunction : public ModelObject {
  EvaluationState es_;
  // later make things implement inputs and return restraints
 public:
  typedef std::pair<double, bool> ScoreIsGoodPair;

 protected:
  /** Do the actual work of computing the score and (optional)
      derivatives. The list of all score states that must be updated
      is passed.*/
  virtual void do_add_score_and_derivatives(ScoreAccumulator sa,
                                            const ScoreStatesTemp &ss) = 0;
  ScoreAccumulator get_score_accumulator_if_below(bool deriv, double max) {
    return ScoreAccumulator(&es_, 1.0, deriv, max, NO_MAX, true);
  }
  ScoreAccumulator get_score_accumulator_if_good(bool deriv) {
    return ScoreAccumulator(&es_, 1.0, deriv, NO_MAX, NO_MAX, true);
  }
  ScoreAccumulator get_score_accumulator(bool deriv) {
    return ScoreAccumulator(&es_, 1.0, deriv, NO_MAX, NO_MAX, false);
  }

 public:
  ScoringFunction(kernel::Model *m, std::string name);

  virtual ModelObjectsTemp do_get_outputs() const IMP_OVERRIDE {
    return ModelObjectsTemp();
  }

  double evaluate_if_good(bool derivatives);

  //! Evaluate and return the score
  /** \return the resulting score

      @param derivatives if true, updates the derivatives of the
                         scoring function
  */
  double evaluate(bool derivatives);

  double evaluate_if_below(bool derivatives, double max);

  /** Return true if the last evaluate satisfied all the restraint
      thresholds.*/
  bool get_had_good_score() const { return es_.good; }

  //! returns the score that was calculated in the last evaluate call
  double get_last_score() const { return es_.score; }
  //! Return a set of restraints equivalent to this scoring function.
  virtual Restraints create_restraints() const = 0;
};

/** Return a list of ScoringFunction objects where each is as simple
    as possible and evaluating the sum (and anding the good score bits)
    is exactly like evaluating the one ScoringFunction.*/
IMPKERNELEXPORT ScoringFunctions create_decomposition(ScoringFunction *sf);

/** This class is to provide a consistent interface for things
    that take ScoringFunctions as arguments.

    \note Passing an empty list of restraints should be supported, but problems
    could arise, so be alert (the problems would not be subtle).
*/
class IMPKERNELEXPORT ScoringFunctionAdaptor :
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
    public base::PointerMember<ScoringFunction>
#else
    public base::InputAdaptor
#endif
    {
  typedef base::PointerMember<ScoringFunction> P;
  static ScoringFunction *get(ScoringFunction *sf) { return sf; }

  /**
     returns a scoring function that sums a list of restraints.
     If the list is empty, returns a null scoring function
     that always returns 0.
   */
  static ScoringFunction *get(const RestraintsTemp &sf);

  /**
     returns a scoring function that sums a list of restraints.
     If the list is empty, returns a null scoring function
     that always returns 0.
   */
  static ScoringFunction *get(const Restraints &sf);
  static ScoringFunction *get(kernel::Model *sf);
  static ScoringFunction *get(Restraint *sf);

 public:
  ScoringFunctionAdaptor() {}
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  template <class T>
  ScoringFunctionAdaptor(base::internal::PointerBase<T> t)
      : P(get(t)) {}
#endif
  ScoringFunctionAdaptor(ScoringFunction *sf) : P(sf) {}
  ScoringFunctionAdaptor(const RestraintsTemp &sf) : P(get(sf)) {}
  ScoringFunctionAdaptor(const Restraints &sf) : P(get(sf)) {}
  ScoringFunctionAdaptor(Restraint *sf) : P(get(sf)) {}
#ifndef IMP_DOXYGEN
  ScoringFunctionAdaptor(kernel::Model *sf) : P(get(sf)) {}
#endif
};

//! Print the hierarchy of restraints
/** The maximum accepted score (Restraint::get_maximum_score())
    and the weight (Restraint::get_weight()) are printed for each restraint.*/
IMPKERNELEXPORT void show_restraint_hierarchy(ScoringFunctionAdaptor rs,
                                              std::ostream &out = std::cout);

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_SCORING_FUNCTION_H */
