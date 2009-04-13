/**
 *  \file converters.cpp
 *  \brief impl
 *  Copyright 2007-9 Sali Lab. All rights reserved.
*/

#include <IMP/em/converters.h>

IMPEM_BEGIN_NAMESPACE

FloatKey get_density_key() {
  static FloatKey dv("density_val");
  return dv;
}

IMPEM_END_NAMESPACE
