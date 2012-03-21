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

//! Define the basic things you need for a Restraint.
/** In addition to the methods declared and defined by IMP_OBJECT
    it declares
    - IMP::Restraint::unprotected_evaluate()
    - IMP::Restraint::get_input_containers()
    - IMP::Restraint::get_input_particles()
*/
#define IMP_RESTRAINT(Name)                                             \
  public:                                                               \
  IMP_IMPLEMENT( double                                                 \
                unprotected_evaluate(DerivativeAccumulator *accum) const); \
  IMP_IMPLEMENT_INLINE(ScoringFunction *                                \
                       create_scoring_function(double weight=1.0,       \
                                               double max               \
                                               = NO_MAX) const, {       \
                         return IMP::internal::create_scoring_function  \
                             (const_cast<Name*>(this),                  \
                              weight, max);                             \
                       });                                              \
  IMP_IMPLEMENT(ContainersTemp get_input_containers() const);           \
  IMP_IMPLEMENT(ParticlesTemp get_input_particles() const);             \
  IMP_OBJECT(Name)

#endif  /* IMPKERNEL_RESTRAINT_MACROS_H */
