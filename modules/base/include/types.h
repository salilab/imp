/**
 *  \file IMP/base/types.h    \brief Basic types used by IMP.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_TYPES_H
#define IMPBASE_TYPES_H

#include "base_config.h"
#include "base_macros.h"
#include <IMP/compatibility/hash.h>
#include <IMP/compatibility/nullptr.h>

//IMPBASE_BEGIN_NAMESPACE
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

namespace base {
//! The log levels supported by \imp
/**
    DEFAULT is only local logging (like in IMP::Object); it means to use
    the global log level.

    VERBOSE prints very large amounts of information. It should be enough
    to allow the computational flow to be understood.

    TERSE prints a few lines per restraint or per state each time
    the score is evaluated.

    PROGRESS show how large calculations are progressing (eg samplers)
    but otherwise just show warnings

    WARNING prints only warnings.

    MEMORY prints information about allocations and deallocations to debug
    memory issues.
 */
enum LogLevel {DEFAULT=-1, SILENT=0, WARNING=1, PROGRESS=2,
               TERSE=3, VERBOSE=4,
               MEMORY=5
#ifndef IMP_DOXYGEN
               , ALL_LOG
#endif
};

//! Determine the level of runtime checks performed
/** - NONE means that minimial checks are
    used.
    - USAGE means that checks of input values to functions
    and classes are verified.
    - USAGE_AND_INTERNAL adds checks that \imp itself is
    correct. Turn these on if you suspect an \imp bug or are
    developing Restraints or other \imp classes.
*/
enum CheckLevel {DEFAULT_CHECK=-1, NONE=0, USAGE=1, USAGE_AND_INTERNAL=2};
}
}
  //IMPBASE_END_NAMESPACE

#endif  /* IMPBASE_TYPES_H */
