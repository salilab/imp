/**
 *  \file Typed.cpp   \brief Simple xyz decorator.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/Typed.h>

IMPCORE_BEGIN_NAMESPACE

IntKey Typed::get_type_key() {
  static IntKey k("particle type");
  return k;
}
IMPCORE_END_NAMESPACE
