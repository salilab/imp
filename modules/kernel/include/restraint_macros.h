/**
 *  \file IMP/kernel/restraint_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_RESTRAINT_MACROS_H
#define IMPKERNEL_RESTRAINT_MACROS_H
#include <IMP/kernel/kernel_config.h>
#include <IMP/base/doxygen_macros.h>
#include <IMP/base/deprecation_macros.h>
#include "internal/scoring_functions.h"
#include "container_base.h"
#include "input_output_macros.h"
#include "constants.h"

/** To implement a new restraint, just implement the two methods:
- IMP::Restraint::do_add_score_and_derivatives()
  (or IMP::Restraint::unprotected_evaluate())
- IMP::ModelObjectsTemp::do_get_inputs();
and use the macro to handle IMP::base::Object
- IMP_OBJECT_METHODS()
*/
#define IMP_RESTRAINT(Name)                                             \
  public:                                                               \
  IMP_IMPLEMENT( double                                                 \
          unprotected_evaluate(IMP::kernel::DerivativeAccumulator *accum) \
                 const);                                                \
  IMP_MODEL_OBJECT_BACKWARDS_MACRO_INPUTS;                              \
  IMP_OBJECT(Name)

#if IMP_HAS_DEPRECATED
//! For backwards compatibility
#define IMP_RESTRAINT_2(Name)                                           \
  public:                                                               \
  IMP_IMPLEMENT( double                                                 \
         unprotected_evaluate(IMP::kernel::DerivativeAccumulator *accum) \
                 const);                                                \
  IMP_IMPLEMENT(IMP::kernel::ModelObjectsTemp do_get_inputs() const);   \
  IMP_OBJECT(Name)

//! For backwards compatability
#define IMP_RESTRAINT_ACCUMULATOR(Name)                                 \
  public:                                                               \
  void do_add_score_and_derivatives(IMP::kernel::ScoreAccumulator sa)   \
  const;                                                                \
  IMP::kernel::ModelObjectsTemp do_get_inputs() const;                  \
  IMP_OBJECT(Name)
#endif

#endif  /* IMPKERNEL_RESTRAINT_MACROS_H */
