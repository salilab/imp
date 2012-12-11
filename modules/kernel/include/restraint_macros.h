/**
 *  \file IMP/restraint_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_RESTRAINT_MACROS_H
#define IMPKERNEL_RESTRAINT_MACROS_H
#include "kernel_config.h"
#include <IMP/base/doxygen_macros.h>
#include "internal/scoring_functions.h"
#include "container_base.h"
#include "input_output_macros.h"
#include "constants.h"

/** At this point, you should probably use IMP_RESTRAINT_ACCUMULATOR()
    for new restraints.
*/
#define IMP_RESTRAINT(Name)                                             \
  public:                                                               \
  IMP_IMPLEMENT( double                                                 \
                 unprotected_evaluate(IMP::DerivativeAccumulator *accum)\
                 const);                                                \
  IMP_IMPLEMENT_INLINE(ScoringFunction *                                \
                       create_scoring_function(double weight=1.0,       \
                                               double max               \
                                               = IMP::NO_MAX) const, {  \
                         set_was_used(true);                            \
                         return IMP::internal::create_scoring_function  \
                             (const_cast<Name*>(this),                  \
                              weight, max);                             \
                       });                                              \
  IMP_MODEL_OBJECT_BACKWARDS_MACRO_INPUTS;                              \
  IMP_OBJECT(Name)

//! Define the basic things you need for a Restraint.
/** In addition to the methods declared and defined by IMP_OBJECT
    it declares
    - IMP::Restraint::unprotected_evaluate()
    - IMP::Restraint::get_inputs()
*/
#define IMP_RESTRAINT_2(Name)                                             \
  public:                                                               \
  IMP_IMPLEMENT( double                                                 \
                 unprotected_evaluate(IMP::DerivativeAccumulator *accum) \
                 const);                                                \
  IMP_IMPLEMENT_INLINE(IMP::ScoringFunction *                           \
                       create_scoring_function(double weight=1.0,       \
                                               double max               \
                                               = IMP::NO_MAX) const, {  \
                         return IMP::internal::create_scoring_function  \
                             (const_cast<Name*>(this),                  \
                              weight, max);                             \
                       });                                              \
  IMP_IMPLEMENT(IMP::ModelObjectsTemp do_get_inputs() const);           \
  IMP_OBJECT(Name)

//! Define the basic things you need for a Restraint.
/** In addition to the methods declared and defined by IMP_OBJECT()
    it declares
    - IMP::Restraint::do_add_score_and_derivatives()
    - IMP::Restraint::do_get_inputs()
*/
#define IMP_RESTRAINT_ACCUMULATOR(Name)                                 \
  public:                                                               \
  IMP_IMPLEMENT( void                                                   \
                 do_add_score_and_derivatives(IMP::ScoreAccumulator sa) \
                 const);                                                \
  IMP_IMPLEMENT_INLINE(IMP::ScoringFunction *                           \
                       create_scoring_function(double weight=1.0,       \
                                               double max               \
                                               = IMP::NO_MAX) const, {  \
                         return IMP::internal::create_scoring_function  \
                             (const_cast<Name*>(this),                  \
                              weight, max);                             \
                       });                                              \
  IMP_IMPLEMENT(ModelObjectsTemp do_get_inputs() const);                \
  IMP_OBJECT(Name)

#endif  /* IMPKERNEL_RESTRAINT_MACROS_H */
