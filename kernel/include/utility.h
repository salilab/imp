/**
 *  \file IMP/utility.h    \brief Various general useful functions for IMP.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_UTILITY_H
#define __IMP_UTILITY_H

#include "macros.h"

namespace IMP
{

//! Compute the square of a number
template <class T>
T square(T t)
{
  return t*t;
}

} // namespace IMP

#endif  /* __IMP_UTILITY_H */
