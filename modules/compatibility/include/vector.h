/**
 *  \file IMP/compatibility/vector.h
 *  \brief Declare a bounds checked vector
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPCOMPATIBILITY_VECTOR_H
#define IMPCOMPATIBILITY_VECTOR_H

#include "compatibility_config.h"

#if IMP_BUILD < IMP_RELEASE && defined(__GNUC__)
#include <debug/vector>
#else
#include <vector>
#endif


IMPCOMPATIBILITY_BEGIN_NAMESPACE
#ifdef SWIG
template <class T>
class vector: public std::vector<T> {};

#elif defined(IMP_DOXYGEN)
/** This class defines a std::vector-like template that, when available
    and IMP is not built in fast mode, performs bounds checking. In general
    IMP code should prefer this vector to std::vector for internal storage.

    Currently, bounds checking is provided when compiling with gcc.
*/
typedef std::vector vector;

#else

#if IMP_BUILD < IMP_RELEASE && defined(__GNUC__)
using __gnu_debug::vector;
#else
using std::vector;
#endif

#endif

IMPCOMPATIBILITY_END_NAMESPACE

#endif  /* IMPCOMPATIBILITY_VECTOR_H */
