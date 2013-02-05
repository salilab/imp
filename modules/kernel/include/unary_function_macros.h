/**
 *  \file IMP/kernel/unary_function_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_UNARY_FUNCTION_MACROS_H
#define IMPKERNEL_UNARY_FUNCTION_MACROS_H
#include <IMP/kernel/kernel_config.h>
#include "UnaryFunction.h"



//! Do not use
#define IMP_UNARY_FUNCTION(Name)                                        \
  IMP_IMPLEMENT(virtual DerivativePair                                  \
                evaluate_with_derivative(double feature) const);        \
  IMP_IMPLEMENT(virtual double evaluate(double feature) const);         \
  IMP_OBJECT(Name)



//! Do not use.
#define IMP_UNARY_FUNCTION_INLINE(Name, value_expression,               \
                                  derivative_expression, show_expression) \
  IMP_IMPLEMENT_INLINE(virtual DerivativePair                           \
  evaluate_with_derivative(double feature) const, {                     \
    return DerivativePair((value_expression), (derivative_expression)); \
                       });                                              \
  IMP_IMPLEMENT_INLINE(virtual double evaluate(double feature) const,{  \
    return (value_expression);                                          \
    });                                                                 \
  IMP_OBJECT_METHODS(Name)



#endif  /* IMPKERNEL_UNARY_FUNCTION_MACROS_H */
