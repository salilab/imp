/**
 *  \file IMP/example/ExampleUnaryFunction.h
 *  \brief A simple unary function.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEXAMPLE_EXAMPLE_UNARY_FUNCTION_H
#define IMPEXAMPLE_EXAMPLE_UNARY_FUNCTION_H

#include <IMP/example/example_config.h>
#include <IMP/UnaryFunction.h>
#include <IMP/unary_function_macros.h>
#include <IMP/utility.h>

IMPEXAMPLE_BEGIN_NAMESPACE

//! A simple unary function
/** This one happens to be a harmonic.
    The source code is as follows:
    \include ExampleUnaryFunction.h

    \note The class does not have an IMPEXAMPLEEXPORT
    since it is all defined in a header.
 */
class ExampleUnaryFunction : public UnaryFunction
{
  Float center_;
  Float k_;
public:
  /** Create with the given center and spring constant. While it
      is generally bad form to have two Float arguments, it is
      hard to avoid here, and there is a bit of a sanity check.*/
  ExampleUnaryFunction(Float center, Float k) : center_(center), k_(k) {
    IMP_USAGE_CHECK(k > 0, "The spring constant must be positive.");
  }

  virtual DerivativePair evaluate_with_derivative(double feature)
  const IMP_OVERRIDE {
    return DerivativePair(evaluate(feature), k_*(feature - center_));
  }
  virtual double evaluate(double feature) const IMP_OVERRIDE {
    return .5*k_*algebra::get_squared(feature - center_);
  }
  IMP_OBJECT_METHODS(ExampleUnaryFunction);
};

IMPEXAMPLE_END_NAMESPACE

#endif  /* IMPEXAMPLE_EXAMPLE_UNARY_FUNCTION_H */
