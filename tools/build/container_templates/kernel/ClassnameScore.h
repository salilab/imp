/**
 *  \file IMP/kernel/ClassnameScore.h
 *  \brief Define ClassnameScore.
 *
 *  BLURB
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_CLASSNAME_SCORE_H
#define IMPKERNEL_CLASSNAME_SCORE_H

#include <IMP/kernel/kernel_config.h>
#include "base_types.h"
#include "ParticleTuple.h"
#include "DerivativeAccumulator.h"
#include "internal/container_helpers.h"
#include <IMP/base/utility_macros.h>
#include "model_object_helpers.h"

IMPKERNEL_BEGIN_NAMESPACE

//! Abstract class for scoring object(s) of type TYPENAME
/** ClassnameScore will evaluate the score and derivatives
    for passed object(s) of type TYPENAME.

    Use in conjunction with various
    restraints such as IMP::core::ClassnamesRestraint or
    IMP::core::ClassnameRestraint. The restraints couple the score
    functions with appropariate lists of object(s) of type TYPENAME.

    Implementers should check out IMP_CLASSNAME_SCORE().

    \see PredicateClassnameRestraint
*/
class IMPKERNELEXPORT ClassnameScore : public ParticleInputs,
                                       public base::Object {
 public:
  typedef VARIABLETYPE Argument;
  typedef INDEXTYPE IndexArgument;
  typedef ARGUMENTTYPE PassArgument;
  typedef PASSINDEXTYPE PassIndexArgument;
  typedef ClassnameModifier Modifier;
  ClassnameScore(std::string name = "ClassnameScore %1%");
  //! Compute the score and the derivative if needed.
  /** \deprecated_at{2.1} Use the index-based evaluate instead. */
  IMPKERNEL_DEPRECATED_METHOD_DECL(2.1)
  virtual double evaluate(ARGUMENTTYPE vt, DerivativeAccumulator *da) const;

  //! Compute the score and the derivative if needed.
  /** evaluate the score and the derivative if needed over vt

      @param m the model of vt
      @param vt the index in m of an object of type TYPENAME
      @param da a derivative accumulator that reweighting
                computed derivatives. If nullptr, derivatives
                will not be computed
   */
  virtual double evaluate_index(kernel::Model *m, PASSINDEXTYPE vt,
                                DerivativeAccumulator *da) const;

  //! Compute the score and the derivative if needed over a set.
  /** evaluate the score and the derivative if needed over a set
      of objects in o

      @param m the model of o
      @param o objects of type TYPENAME, specified by index
      @param da a derivative accumulator that reweighting
                computed derivatives. If nullptr, derivatives
                will not be computed
      @param lower_bound index of first item in o to evaluate
      @param upper_bound index of last item in o to evaluate

      @note Implementations
      for these are provided by the IMP_CLASSNAME_SCORE()
      macro.
  */
  virtual double evaluate_indexes(kernel::Model *m, const PLURALINDEXTYPE &o,
                                  DerivativeAccumulator *da,
                                  unsigned int lower_bound,
                                  unsigned int upper_bound) const;

  //! Compute the score and the derivative if needed.
  /** Compute the score and the derivative if needed as in evaluate_index().
      but may terminate the computation early if the score is higher than max.

      @return the score if score<= max or some arbitrary value > max otherwise.
  */
  virtual double evaluate_if_good_index(kernel::Model *m, PASSINDEXTYPE vt,
                                        DerivativeAccumulator *da,
                                        double max) const;

  /** Compute the score and the derivative if needed as in evaluate_index().
      but may terminate the computation early if the total score is higher than max.

      @return the score if score<= max or some arbitrary value > max otherwise.

      Implementations
      for these are provided by the IMP_CLASSNAME_SCORE()
      macro.
  */
  virtual double evaluate_if_good_indexes(kernel::Model *m,
                                          const PLURALINDEXTYPE &o,
                                          DerivativeAccumulator *da, double max,
                                          unsigned int lower_bound,
                                          unsigned int upper_bound) const;
  /** Decompose this pair score acting on the pair into a set of
      restraints. The scoring function and derivatives should
      be equal to the current score. The defualt implementation
      just returns this object bound to the pair.*/
  Restraints create_current_decomposition(kernel::Model *m,
                                          PASSINDEXTYPE vt) const;

 protected:
  /** Overide this to return your own decomposition.*/
  virtual Restraints do_create_current_decomposition(kernel::Model *m,
                                                     PASSINDEXTYPE vt) const;

  IMP_REF_COUNTED_DESTRUCTOR(ClassnameScore);
};

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_CLASSNAME_SCORE_H */
