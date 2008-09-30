/**
 *  \file IMP/utility.h    \brief Various general useful functions for IMP.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_UTILITY_H
#define __IMP_UTILITY_H

#include "macros.h"

IMP_BEGIN_NAMESPACE

//! Compute the square of a number
template <class T>
T square(T t)
{
  return t*t;
}

IMP_END_NAMESPACE

#endif  /* __IMP_UTILITY_H */
