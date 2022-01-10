/**
 *  \file IMP/ScoringFunction.h
 *  \brief Represents a scoring function on the model.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_SCORING_FUNCTION_H
#define IMPKERNEL_SCORING_FUNCTION_H

#include <IMP/kernel_config.h>
#include "base_types.h"
#include "dependency_graph.h"
#include "Restraint.h"
#include "ModelObject.h"
#include "internal/moved_particles_cache.h"
#include <IMP/InputAdaptor.h>
#include <IMP/Pointer.h>

#include <limits>

IMPKERNEL_BEGIN_NAMESPACE
class Model;

//! Represents a scoring function on the model.
/**
A call to the evaluate() method prompts the following events:
1. determine set of ScoreState objects needed by the Restraint objects
being evaluated (this is cached)
2. call ScoreState::before_evaluate() on each of them to update
    configuration
3. call Restraint::unprotected_evaluate() to compute scores
    [and add derivatives in the particles, if requested]
4. [call ScoreState::after_evaluate() on each score state to update derivatives]
5. return the score

    \headerfile ScoringFunction.h "IMP/ScoringFunction.h"

*/
class IMPKERNELEXPORT ScoringFunction : public ModelObject {
  EvaluationState es_;
  // cache of ScoreStates that are affected by each moved particle,
  // used for evaluate_moved() and related functions
  internal::MovedParticlesScoreStateCache moved_particles_cache_;
  // time when moved_particles_cache_ was last updated, or 0
  unsigned moved_particles_cache_age_;

  ScoreStatesTemp get_moved_required_score_states(
                               const ParticleIndexes &moved_pis,
                               const ParticleIndexes &reset_pis);

  // later make things implement inputs and return restraints
 public:
  typedef std::pair<double, bool> ScoreIsGoodPair;

 protected:
  /** Do the actual work of computing the score and (optional)
      derivatives. The list of all score states that must be updated
      is passed.*/
  virtual void do_add_score_and_derivatives(ScoreAccumulator sa,
                                            const ScoreStatesTemp &ss) = 0;

  //! Score when only some particles have moved.
  /** \see do_add_score_and_derivatives()
   */
  virtual void do_add_score_and_derivatives_moved(
                  ScoreAccumulator sa, const ParticleIndexes &moved_pis,
                  const ParticleIndexes &reset_pis,
                  const ScoreStatesTemp &ss) {
    IMP_UNUSED(moved_pis);
    IMP_UNUSED(reset_pis);
    do_add_score_and_derivatives(sa, ss);
  }

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
  ScoringFunction(Model *m, std::string name);

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

  //! Score when some particles have moved.
  /** This should behave identically to evaluate() but may be more
      efficient if it can skip restraint terms that involve unchanged particles.

      \param moved_pis Particles that have moved since the last
             scoring function evaluation.
      \param reset_pis Particles that have moved, but back to the
             positions they had at the last-but-one evaluation
             (e.g. due to a rejected Monte Carlo move).

      \see evaluate()
   */
  double evaluate_moved(bool derivatives, const ParticleIndexes &moved_pis,
                        const ParticleIndexes &reset_pis);

  double evaluate_moved_if_below(
             bool derivatives, const ParticleIndexes &moved_pis,
             const ParticleIndexes &reset_pis, double max);

  double evaluate_moved_if_good(
             bool derivatives, const ParticleIndexes &moved_pis,
             const ParticleIndexes &reset_pis);

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
    public PointerMember<ScoringFunction>
#else
    public InputAdaptor
#endif
    {
  typedef PointerMember<ScoringFunction> P;
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
  static ScoringFunction *get(Restraint *sf);

 public:
  ScoringFunctionAdaptor() {}
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  template <class T>
  ScoringFunctionAdaptor(internal::PointerBase<T> t)
      : P(get(t)) {}
#endif
  ScoringFunctionAdaptor(ScoringFunction *sf) : P(sf) {}
  ScoringFunctionAdaptor(const RestraintsTemp &sf) : P(get(sf)) {}
  ScoringFunctionAdaptor(const Restraints &sf) : P(get(sf)) {}
  ScoringFunctionAdaptor(Restraint *sf) : P(get(sf)) {}
};

//! Print the hierarchy of restraints
/** The maximum accepted score (Restraint::get_maximum_score())
    and the weight (Restraint::get_weight()) are printed for each restraint.*/
IMPKERNELEXPORT void show_restraint_hierarchy(ScoringFunctionAdaptor rs,
                                              std::ostream &out = std::cout);

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_SCORING_FUNCTION_H */
