/**
 *  \file isd/ExampleUnaryFunction.h
 *  \brief A simple unary function.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */


#ifndef IMPISD_ISD_UNARY_FUNCTION_H
#define IMPISD_ISD_UNARY_FUNCTION_H

#include "isd_config.h"
#include <IMP/UnaryFunction.h>
#include <IMP/utility.h>

IMPISD_BEGIN_NAMESPACE

//! A simple unary function
/** This one happens to be a harmonic.
    The source code is as follows:
    \include ExampleUnaryFunction.h

    \note The class does not have an IMPISDEXPORT
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

  IMP_UNARY_FUNCTION_INLINE(ExampleUnaryFunction,
                            .5*k_*square(feature-center_),
                            k_*(feature - center_),
                            "Harmonic: " << center_ << " and " << k_
                            << std::endl);
};

IMPISD_END_NAMESPACE

#endif  /* IMPISD_ISD_UNARY_FUNCTION_H */
