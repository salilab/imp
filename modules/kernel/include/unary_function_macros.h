/**
 *  \file IMP/unary_function_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_UNARY_FUNCTION_MACROS_H
#define IMPKERNEL_UNARY_FUNCTION_MACROS_H
#include "kernel_config.h"
#include "UnaryFunction.h"



//! Declare the needed functions for a UnaryFunction
/** In addition to the methods done by all the macros, it declares
    - IMP::UnaryFunction::evaluate()
    - IMP::UnaryFunction::evaluate_with_derivatives()

    \see IMP_UNARY_FUNCTION_INLINE
*/
#define IMP_UNARY_FUNCTION(Name)                                        \
  IMP_IMPLEMENT(virtual DerivativePair                                  \
                evaluate_with_derivative(double feature) const);        \
  IMP_IMPLEMENT(virtual double evaluate(double feature) const);         \
  IMP_OBJECT(Name)



#endif  /* IMPKERNEL_UNARY_FUNCTION_MACROS_H */
