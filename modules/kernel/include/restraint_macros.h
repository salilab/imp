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

/** \deprecated_at{2.1} Declare the methods yourself instead.
*/
#define IMP_RESTRAINT(Name)                                                    \
  IMPKERNEL_DEPRECATED_MACRO(2.1, "Declare the methods yourself");             \
                                                                               \
 public:                                                                       \
  double unprotected_evaluate(IMP::kernel::DerivativeAccumulator *accum) const \
      IMP_OVERRIDE;                                                            \
  IMP_MODEL_OBJECT_BACKWARDS_MACRO_INPUTS;                                     \
  IMP_OBJECT_NO_WARNING(Name)

/**  \deprecated_at{2.1} Declare the methods yourself instead.
 */
#define IMP_RESTRAINT_2(Name)                                               \
 public:                                                                    \
  IMPKERNEL_DEPRECATED_MACRO(2.1, "Declare the methods yourself");          \
  virtual double unprotected_evaluate(                                      \
      IMP::kernel::DerivativeAccumulator *accum) const IMP_OVERRIDE;        \
  virtual IMP::kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE; \
  IMP_OBJECT_NO_WARNING(Name)

/**  \deprecated_at{2.1} Declare the methods yourself instead.
 */
#define IMP_RESTRAINT_ACCUMULATOR(Name)                                      \
 public:                                                                     \
  IMPKERNEL_DEPRECATED_MACRO(2.1, "Declare the methods yourself");           \
  void do_add_score_and_derivatives(IMP::kernel::ScoreAccumulator sa) const; \
  IMP::kernel::ModelObjectsTemp do_get_inputs() const;                       \
  IMP_OBJECT_NO_WARNING(Name)

#endif /* IMPKERNEL_RESTRAINT_MACROS_H */
