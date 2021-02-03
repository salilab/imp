/**
 *  \file IMP/algebra/ReferenceFrame3D.h
 *  \brief A reference frame in 3D.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_REFERENCE_FRAME_3D_H
#define IMPALGEBRA_REFERENCE_FRAME_3D_H

#include <IMP/algebra/algebra_config.h>
#include "Transformation3D.h"

IMPALGEBRA_BEGIN_NAMESPACE

//! A reference frame in 3D.
/**
    \geometry
*/
class IMPALGEBRAEXPORT ReferenceFrame3D {
  Transformation3D tr_;
  mutable bool has_inverse_;
  mutable Transformation3D tri_;
  const Transformation3D &get_inverse() const {
    if (!has_inverse_) {
      tri_ = tr_.get_inverse();
      has_inverse_ = true;
    }
    return tri_;
  }

 public:
  IMP_CXX11_DEFAULT_COPY_CONSTRUCTOR(ReferenceFrame3D);
  //! Create the default reference frame
  /** That is, the origin with x,y,z axis as the principal axes. */
  ReferenceFrame3D()
      : tr_(get_identity_transformation_3d()), has_inverse_(true), tri_(tr_) {}

  //! A reference frame specified by a transformation matrix
  /** Constructs a reference frame using transformation tr

      @param tr a transformation from local coordinates in this
                constructed reference frame to the global coordinates
                (equiv., tr brings the origin of the global frame
                 to the origin of this frame, in global coordinates),
  */
  explicit ReferenceFrame3D(const Transformation3D &tr)
      : tr_(tr), has_inverse_(false) {}
  ~ReferenceFrame3D();
  //! Return transformation from local to global coordinates

  /**
     Get the transformation that brings the origin of the global reference
     frame to the origin of this frame (both in global coordinates).
  */
  const Transformation3D &get_transformation_to() const { return tr_; }

  //! Return the transformation from global to local coordinates
  /**
     Get the transformation from the origin of this reference frame
     to the origin of the global frame (both in global coordinates).
  */
  const Transformation3D &get_transformation_from() const {
    return get_inverse();
  }

  //! Convert the vector v from local to global coordinates
  //! (i.e., v is specified in this reference frame)
  Vector3D get_global_coordinates(const Vector3D &v) const {
    return tr_.get_transformed(v);
  }

  //! Convert the vector v from global to local coordinates
  //! (i.e., the returned vector is specified in this reference frame)
  Vector3D get_local_coordinates(const Vector3D &v) const {
    return get_inverse().get_transformed(v);
  }

  //! Convert reference frame v from local to global
  //! (i.e., v is specified in this reference frame)
  ReferenceFrame3D get_global_reference_frame(const ReferenceFrame3D &v) const {
    return ReferenceFrame3D(tr_ * v.tr_);
  }

  //! Convert reference frame v from global to local
  //! (i.e. the returned reference frame is speficied in this reference frame)
  ReferenceFrame3D get_local_reference_frame(const ReferenceFrame3D &v) const {
    return ReferenceFrame3D(get_inverse() * v.tr_);
  }

  IMP_SHOWABLE_INLINE(ReferenceFrame3D, { out << tr_; });
};

IMP_VALUES(ReferenceFrame3D, ReferenceFrame3Ds);

inline ReferenceFrame3D get_transformed(const ReferenceFrame3D &rf,
                                        const Transformation3D &tr) {
  return ReferenceFrame3D(tr * rf.get_transformation_to());
}

inline Transformation3D get_transformation_from_first_to_second(
    const ReferenceFrame3D &a, const ReferenceFrame3D &b) {
  return b.get_transformation_to() * a.get_transformation_from();
}

IMPALGEBRA_END_NAMESPACE
#endif /* IMPALGEBRA_REFERENCE_FRAME_3D_H */
