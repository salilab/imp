/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/CoordinateTransformer.h>
#include <RMF/decorators.h>

RMF_ENABLE_WARNINGS

RMF_VECTOR_DEF(CoordinateTransformer);

namespace RMF {
CoordinateTransformer::CoordinateTransformer(CoordinateTransformer base,
                                             ReferenceFrameConst   rb):
  transform_(base.transform_,
             internal::Transform(internal::Rotation(rb.get_rotation()),
                                 rb.get_translation())) {
}
}

RMF_DISABLE_WARNINGS
