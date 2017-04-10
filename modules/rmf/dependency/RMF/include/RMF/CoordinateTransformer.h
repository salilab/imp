/**
 *  \file RMF/CoordinateTransformer.h
 *  \brief Declare the RMF::CoordinateTransformer class.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_COORDINATE_TRANSFORMER_H
#define RMF_COORDINATE_TRANSFORMER_H

#include <vector>

#include "RMF/Vector.h"
#include "RMF/compiler_macros.h"
#include "RMF/config.h"
#include "RMF/internal/Transform.h"
#include "infrastructure_macros.h"

RMF_ENABLE_WARNINGS
namespace RMF {
namespace decorator {
class ReferenceFrameConst;
}

//! Transform coordinates into the global reference frame
/** Transform coordinates into the global reference frame from
    a nested one.
 */
class RMFEXPORT CoordinateTransformer {
  internal::Transform transform_;

 public:
  /** Create one with the default reference frame, so the points aren't
      transformed at all.*/
  CoordinateTransformer() {};
  /** Compose an outer CoordinateTransformer with the nested ReferenceFrame.*/
  CoordinateTransformer(CoordinateTransformer base,
                        decorator::ReferenceFrameConst rb);
  RMF_SHOWABLE(CoordinateTransformer, transform_);
  /** Return the global coordinates for a given set of internal coordinates.
   */
  Vector3 get_global_coordinates(const Vector3& internal) const {
    return transform_.get_transformed(internal);
  }

  /** Return the translational component of the transformation.*/
  const Vector3& get_translation() const {
    return transform_.get_translation();
  }
  /** Return the rotational component of the transformation as a quaternion.*/
  const Vector4& get_rotation() const { return transform_.get_rotation(); }
};

//! A list of them
typedef std::vector<CoordinateTransformer> CoordinateTransformers;

} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_COORDINATE_TRANSFORMER_H */
