/**
 *  \file IMP/example/ExampleUnaryFunction.h
 *  \brief A simple unary function.
 *
 *  Copyright 2007-2023 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEXAMPLE_EXAMPLE_UNARY_FUNCTION_H
#define IMPEXAMPLE_EXAMPLE_UNARY_FUNCTION_H

#include <IMP/example/example_config.h>
#include <IMP/UnaryFunction.h>
#include <IMP/utility.h>
#include <IMP/algebra/utility.h>
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/types/polymorphic.hpp>

IMPEXAMPLE_BEGIN_NAMESPACE

//! A simple unary function
/** This one happens to be a harmonic.
    The source code is as follows:
    \include ExampleUnaryFunction.h
 */
class IMPEXAMPLEEXPORT ExampleUnaryFunction : public UnaryFunction {
  Float center_;
  Float k_;

 public:
  /** Create with the given center and spring constant. While it
      is generally bad form to have two Float arguments, it is
      hard to avoid here, and there is a bit of a sanity check.*/
  ExampleUnaryFunction(Float center, Float k) : center_(center), k_(k) {
    IMP_USAGE_CHECK(k > 0, "The spring constant must be positive.");
  }

  ExampleUnaryFunction() {}

  virtual DerivativePair evaluate_with_derivative(double feature) const
      override {
    return DerivativePair(evaluate(feature), k_ * (feature - center_));
  }
  virtual double evaluate(double feature) const override {
    return .5 * k_ * algebra::get_squared(feature - center_);
  }
  IMP_OBJECT_METHODS(ExampleUnaryFunction);

 private:
  friend class cereal::access;
  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<UnaryFunction>(this), center_, k_);
  }
  IMP_OBJECT_SERIALIZE_DECL(ExampleUnaryFunction);
};

IMPEXAMPLE_END_NAMESPACE

#endif /* IMPEXAMPLE_EXAMPLE_UNARY_FUNCTION_H */
