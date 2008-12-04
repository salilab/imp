/**
 *  \file Rotation3D.cpp   \brief Simple 3D rotation class.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */
#include "IMP/core/Rotation3D.h"
IMPCORE_BEGIN_NAMESPACE

Rotation3D::~Rotation3D() {
}

Rotation3D Rotation3D::get_inverse() const {
  IMP_check(a_ != 0 || b_ != 0 || c_ != 0 || d_ != 0,
            "Attempting to invert uninitialized rotation",
            InvalidStateException);
  Rotation3D ret(a_, -b_, -c_, -d_);
  return ret;
}

IMPCORE_END_NAMESPACE
