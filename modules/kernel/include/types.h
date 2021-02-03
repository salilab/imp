/**
 *  \file IMP/types.h    \brief Basic types used by IMP.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_TYPES_H
#define IMPKERNEL_TYPES_H

#include <IMP/kernel_config.h>
#include "value_macros.h"
#include "object_macros.h"
#include <IMP/hash.h>
#include <IMP/nullptr.h>

// IMPKERNEL_BEGIN_NAMESPACE
namespace IMP {
//! Basic floating-point value (could be float, double...)
typedef double Float;

//! A pair representing a function value with its first derivative
typedef std::pair<double, double> DerivativePair;
IMP_BUILTIN_VALUES(DerivativePair, DerivativePairs);

//! A generic pair of floats
typedef std::pair<double, double> FloatPair;
IMP_BUILTIN_VALUES(FloatPair, FloatPairs);

//! A pair representing the allowed range for a Float attribute
typedef std::pair<Float, Float> FloatRange;
IMP_BUILTIN_VALUES(FloatRange, FloatRanges);

//! Basic integer value
typedef int Int;

typedef std::pair<Int, Int> IntRange;
IMP_BUILTIN_VALUES(IntRange, IntRanges);

typedef std::pair<Int, Int> IntPair;
IMP_BUILTIN_VALUES(IntPair, IntPairs);

//! Basic string value
typedef std::string String;

//! Standard way to pass a bunch of Float values
IMP_BUILTIN_VALUES(Float, Floats);
//! Standard way to pass a bunch of Int values
IMP_BUILTIN_VALUES(Int, Ints);
//! Standard way to pass a bunch of String values
IMP_BUILTIN_VALUES(String, Strings);

//! Standard way to pass a bunch of Floats values
IMP_BUILTIN_VALUES(Floats, FloatsList);
//! Standard way to pass a bunch of Ints values
IMP_BUILTIN_VALUES(Ints, IntsList);
//! Standard way to pass a bunch of Strings values
IMP_BUILTIN_VALUES(Strings, StringsList);

class Object;
//! A list of objects
IMP_OBJECTS(Object, Objects);
}
// IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_TYPES_H */
