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



//! Declare the needed functions for a UnaryFunction which evaluates inline
/** This macro declares all the functions needed for an IMP::UnaryFunction
    inline in the class. There is no need for an associated \c .cpp file.

    The last three arguments are expressions that evaluate to the
    unary function value and derivative and are sent to the stream in the
    show function, respectively. The input to the function is called
    \c feature.

    \see IMP_UNARY_FUNCTION
*/
#define IMP_UNARY_FUNCTION_INLINE(Name, value_expression,               \
                                  derivative_expression, show_expression) \
  IMP_IMPLEMENT_INLINE(virtual DerivativePair                           \
  evaluate_with_derivative(double feature) const, {                     \
    return DerivativePair((value_expression), (derivative_expression)); \
                       });                                              \
  IMP_IMPLEMENT_INLINE(virtual double evaluate(double feature) const,{  \
    return (value_expression);                                          \
    });                                                                 \
  IMP_OBJECT_INLINE(Name, out << show_expression, {})



#endif  /* IMPKERNEL_UNARY_FUNCTION_MACROS_H */
