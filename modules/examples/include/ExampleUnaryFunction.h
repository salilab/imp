/**
 *  \file ExampleUnaryFunction.h
 *  \brief A simple unary function.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */


#ifndef IMPEXAMPLES_EXAMPLE_UNARY_FUNCTION_H
#define IMPEXAMPLES_EXAMPLE_UNARY_FUNCTION_H

#include "config.h"
#include <IMP/UnaryFunction.h>
#include <IMP/utility.h>

IMPEXAMPLES_BEGIN_NAMESPACE

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
    IMP_check(k > 0, "The spring constant must be positive.",
              ValueException);
  }

  IMP_UNARY_FUNCTION_INLINE(ExampleUnaryFunction,
                            internal::version_info,
                            .5*k_*square(feature-center_),
                            k_*(feature - center_),
                            "Harmonic: " << center_ << " and " << k_
                            << std::endl);
};

IMPEXAMPLES_END_NAMESPACE

#endif  /* IMPEXAMPLES_EXAMPLE_UNARY_FUNCTION_H */
