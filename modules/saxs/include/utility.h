/**
 *  \file saxs/utility.h
 *  \brief Functions to deal with very common math operations
 *  \author Javier Velazquez-Muriel
 *  Copyright 2007-8 Sali Lab. All rights reserved.
*/

#ifndef IMPSAXS_UTILITY_H
#define IMPSAXS_UTILITY_H

#include "config.h"
#include <IMP/exception.h>

IMPSAXS_BEGIN_NAMESPACE

inline Float sinc(Float value) {
  if(fabs(value) < 1.0e-16) return 1.0;
  return sin(value)/value;
}

IMPSAXS_END_NAMESPACE

#endif  /* IMPSAXS_UTILITY_H */
