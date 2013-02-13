/**
 *  \file IMP/kernel/ClassnameScore.h
 *  \brief Define ClassnameScore.
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_CLASSNAME_SCORE_H
#define IMPKERNEL_CLASSNAME_SCORE_H

#include <IMP/kernel/kernel_config.h>
#include "base_types.h"
#include "ParticleTuple.h"
#include "DerivativeAccumulator.h"
#include "internal/container_helpers.h"
#include <IMP/base/utility_macros.h>
#include "input_output_macros.h"

IMPKERNEL_BEGIN_NAMESPACE

//! Abstract score function
/** ClassnameScores will evaluate the score and derivatives
    for the passed particles. Use in conjunction with various
    restraints such as IMP::core::ClassnamesRestraint or
    IMP::core::ClassnameRestraint.

    Implementers should check out IMP_CLASSNAME_SCORE().
*/
class IMPKERNELEXPORT ClassnameScore : public base::Object
{
 public:
  typedef VARIABLETYPE Argument;
  typedef INDEXTYPE IndexArgument;
  typedef ARGUMENTTYPE PassArgument;
  typedef PASSINDEXTYPE PassIndexArgument;
  typedef ClassnameModifier Modifier;
  ClassnameScore(std::string name="ClassnameScore %1%");
  //! Compute the score and the derivative if needed.
  IMP_DEPRECATED_WARN
    virtual double evaluate(ARGUMENTTYPE vt,
                            DerivativeAccumulator *da) const;

  //! Compute the score and the derivative if needed.
  virtual double evaluate_index(Model *m, PASSINDEXTYPE vt,
                                DerivativeAccumulator *da) const;

  /** Implementations
      for these are provided by the IMP_CLASSNAME_SCORE()
      macro.
  */
  virtual double evaluate_indexes(Model *m,
                                  const PLURALINDEXTYPE &o,
                                  DerivativeAccumulator *da,
                                  unsigned int lower_bound,
                                  unsigned int upper_bound) const ;

  //! Compute the score and the derivative if needed.
  virtual double evaluate_if_good_index(Model *m,
                                        PASSINDEXTYPE vt,
                                        DerivativeAccumulator *da,
                                        double max)  const;

  /** Implementations
      for these are provided by the IMP_CLASSNAME_SCORE()
      macro.
  */
  virtual double evaluate_if_good_indexes(Model *m,
                                          const PLURALINDEXTYPE &o,
                                          DerivativeAccumulator *da,
                                          double max,
                                          unsigned int lower_bound,
                                          unsigned int upper_bound)
    const;
  /** Decompose this pair score acting on the pair into a set of
      restraints. The scoring function and derivatives should
      be equal to the current score. The defualt implementation
      just returns this object bound to the pair.*/
  Restraints create_current_decomposition(Model *m,
                                          PASSINDEXTYPE vt) const;

  IMP_INPUTS_DECL(ClassnameScore);

protected:
  /** Overide this to return your own decomposition.*/
  virtual Restraints do_create_current_decomposition
                       (Model *m, PASSINDEXTYPE vt)const;

  IMP_REF_COUNTED_DESTRUCTOR(ClassnameScore);
};

IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_CLASSNAME_SCORE_H */
