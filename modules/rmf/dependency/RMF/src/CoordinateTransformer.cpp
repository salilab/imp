/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include "RMF/CoordinateTransformer.h"
#include "RMF/compiler_macros.h"
#include "RMF/decorator/physics.h"

RMF_ENABLE_WARNINGS

namespace RMF {
CoordinateTransformer::CoordinateTransformer(CoordinateTransformer base,
                                             decorator::ReferenceFrameConst rb)
    : transform_(base.transform_,
                 internal::Transform(internal::Rotation(rb.get_rotation()),
                                     rb.get_translation())) {}
}

RMF_DISABLE_WARNINGS
