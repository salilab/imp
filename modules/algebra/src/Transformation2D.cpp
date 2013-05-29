/**
 *  \file Transformation2D.cpp
 *  \brief Simple 2D transformation class.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */
#include "IMP/algebra/Transformation2D.h"
#include "IMP/algebra/geometric_alignment.h"

IMPALGEBRA_BEGIN_NAMESPACE

Transformation2D::~Transformation2D() {}

Transformation2D Transformation2D::get_inverse() const {
  Rotation2D inv_rot = rot_.get_inverse();
  return Transformation2D(inv_rot, -(inv_rot.get_rotated(trans_)));
}

Transformation2D get_transformation_aligning_pair(const Vector2Ds &set_from,
                                                  const Vector2Ds &set_to) {
  IMP_INTERNAL_CHECK(set_from.size() == 2 && set_to.size() == 2,
                     "rigid_align_first_to_second_2d:: The number of points "
                     "in both sets must be 2");
  // v1 and v2 should be similar
  Vector2D v1 = set_from[1] - set_from[0];
  Vector2D v2 = set_to[1] - set_to[0];
  // Build the rotation to obtain vector v1
  Rotation2D R1 = get_rotation_to_x_axis(v1);
  // Build the rotation to obtain vector v2
  Rotation2D R2 = get_rotation_to_x_axis(v2);
  // Obtain the transformation from v1 to v2
  Rotation2D R = compose(R2, R1.get_inverse());
  Vector2D t = set_to[0] - R.get_rotated(set_from[0]);
  Transformation2D T(R, t);
  return T;
}

//!

IMPALGEBRA_END_NAMESPACE
