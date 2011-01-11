/**
 *  \file ReferenceFrame3D.h   \brief Simple 3D rotation class.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_REFERENCE_FRAME_3D_H
#define IMPALGEBRA_REFERENCE_FRAME_3D_H

#include "Transformation3D.h"

IMPALGEBRA_BEGIN_NAMESPACE



//!  A reference frame in 3D.
/**
    \geometry
*/
class IMPALGEBRAEXPORT ReferenceFrame3D {
  Transformation3D tr_, tri_;
  const Transformation3D& get_inverse() const {
    return tri_;
  }
public:
  //! Create the default reference frame
  /** That is, the origin with x,y,z axis as the principle axes.*/
  ReferenceFrame3D(): tr_(get_identity_transformation_3d()),
    tri_(tr_){}
  explicit ReferenceFrame3D(const Transformation3D &tr): tr_(tr),
    tri_(tr.get_inverse()){}
  ~ReferenceFrame3D();
  //! Get the transformation taking the global reference frame to this one.
  const Transformation3D &get_transformation_to() const {
    return tr_;
  }
  //! Get the transformation from this one to the global reference.
  const Transformation3D &get_transformation_from() const {
    return tri_;
  }
  //! Assume the input vector is in local coordinates and transform
  //! it to global ones.
  Vector3D get_global_coordinates(const Vector3D &v) const {
    return tr_.get_transformed(v);
  }
  //! Assume the input vector is in global coordinates and get the local
  //! coordinates.
  Vector3D get_local_coordinates(const Vector3D &v) const {
    return get_inverse().get_transformed(v);
  }
  IMP_SHOWABLE_INLINE(ReferenceFrame3D, {out << tr_;});
};


IMP_VALUES(ReferenceFrame3D, ReferenceFrame3Ds);

inline ReferenceFrame3D
get_transformed(const ReferenceFrame3D &rf, const Transformation3D &tr) {
  return ReferenceFrame3D(tr*rf.get_transformation_to());
}

inline Transformation3D
get_transformation_from_first_to_second(const ReferenceFrame3D &a,
                                        const ReferenceFrame3D &b) {
  return b.get_transformation_to()*a.get_transformation_from();
}

IMPALGEBRA_END_NAMESPACE
#endif  /* IMPALGEBRA_REFERENCE_FRAME_3D_H */
