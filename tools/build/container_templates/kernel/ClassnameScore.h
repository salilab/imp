/**
 *  \file IMP/ClassnameScore.h
 *  \brief Define ClassnameScore.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_CLASSNAME_SCORE_H
#define IMPKERNEL_CLASSNAME_SCORE_H

#include <IMP/kernel_config.h>
#include "base_types.h"
#include "DerivativeAccumulator.h"
#include "internal/container_helpers.h"
#include <IMP/utility_macros.h>
#include "model_object_helpers.h"

IMPKERNEL_BEGIN_NAMESPACE

//! Abstract class for scoring object(s) of type INDEXTYPE.
/** ClassnameScore will evaluate the score and derivatives
    for passed object(s) of type INDEXTYPE.

    Use in conjunction with various
    restraints such as IMP::container::ClassnamesRestraint or
    IMP::core::ClassnameRestraint. The restraints couple the score
    functions with appropriate lists of object(s) of type INDEXTYPE.

    Implementers should check out IMP_CLASSNAME_SCORE().

    \see PredicateClassnameRestraint
*/
class IMPKERNELEXPORT ClassnameScore : public ParticleInputs,
                                       public Object {
 public:
  typedef VARIABLETYPE Argument;
  typedef INDEXTYPE IndexArgument;
  typedef ARGUMENTTYPE PassArgument;
  typedef PASSINDEXTYPE PassIndexArgument;
  typedef ClassnameModifier Modifier;
  ClassnameScore(std::string name = "ClassnameScore %1%");

  //! Compute the score and the derivative if needed.
  /** @param m the model of vt
      @param vt the index in m of an object of type TYPENAME
      @param da a DerivativeAccumulator that weights
                computed derivatives. If nullptr, derivatives
                will not be computed.
   */
  virtual double evaluate_index(Model *m, PASSINDEXTYPE vt,
                                DerivativeAccumulator *da) const = 0;

  //! Compute the score and the derivative if needed over a set.
  /** @param m the model of o
      @param o objects of type TYPENAME, specified by index
      @param da a derivative accumulator that weights
                computed derivatives. If nullptr, derivatives
                will not be computed.
      @param lower_bound index of first item in o to evaluate
      @param upper_bound index of last item in o to evaluate

      @note Implementations for these are provided by
            the IMP_CLASSNAME_SCORE() macro.
  */
  virtual double evaluate_indexes(Model *m, const PLURALINDEXTYPE &o,
                                  DerivativeAccumulator *da,
                                  unsigned int lower_bound,
                                  unsigned int upper_bound) const;

  //! Compute the score and the derivative if needed, only if "good".
  /** This functions similarly to evaluate_index(),
      but may terminate the computation early if the score is higher than max.

      @return the score if score<= max or some arbitrary value > max otherwise.
  */
  virtual double evaluate_if_good_index(Model *m, PASSINDEXTYPE vt,
                                        DerivativeAccumulator *da,
                                        double max) const;

  /** Compute the score and the derivative if needed over a set, only if "good".
      This functions similarly to evaluate_indexes(), but may terminate
      the computation early if the total score is higher than max.

      @return the score if score<= max or some arbitrary value > max otherwise.

      @note Implementations for these are provided by the IMP_CLASSNAME_SCORE()
            macro.
  */
  virtual double evaluate_if_good_indexes(Model *m,
                                          const PLURALINDEXTYPE &o,
                                          DerivativeAccumulator *da, double max,
                                          unsigned int lower_bound,
                                          unsigned int upper_bound) const;

  //! Decompose this ClassnameScore into a set of
  //! currently positive restraints over vt.
  /** The scoring function and derivatives should
      be equal to the current score. The default implementation
      returns a single restraint with this score bound to vt,
      or zero restraints if the score equals zero.
. */
  Restraints create_current_decomposition(Model *m,
                                          PASSINDEXTYPE vt) const;

 protected:
  //! Override this to return your own decomposition.
  virtual Restraints do_create_current_decomposition(Model *m,
                                                     PASSINDEXTYPE vt) const;

  IMP_REF_COUNTED_DESTRUCTOR(ClassnameScore);
};

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_CLASSNAME_SCORE_H */
