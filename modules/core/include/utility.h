/**
 *  \file core/utility.h    \brief Various important functionality
 *                                       for implementing decorators.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_UTILITY_H
#define IMPCORE_UTILITY_H

#include "macros.h"
#include "core_exports.h"
#include <IMP/base_types.h>

#ifdef __GNUC__
#include <cmath>
#endif


IMPCORE_BEGIN_NAMESPACE

//! Return true if a number is NaN
/** With certain compiler settings the compiler can optimize
    out a!=a (and certain intel chips had issues with it too).
 */
inline bool is_nan(const float& a) {
#ifdef __GNUC__
  return std::isnan(a);
#else
  return a != a;
#endif
}

//! Return true if a number is NaN
/** With certain compiler settings the compiler can optimize
    out a!=a (and certain intel chips had issues with it too).
 */
inline bool is_nan(const double& a) {
#ifdef __GNUC__
  return std::isnan(a);
#else
  return a != a;
#endif
}

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_UTILITY_H */
