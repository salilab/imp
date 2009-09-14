/**
 *  \file Transformation2D.cpp
 *  \brief Simple 2D transformation class.
 *  \author Javier Velazquez-Muriel
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */
#include "IMP/algebra/Transformation2D.h"

IMPALGEBRA_BEGIN_NAMESPACE

Transformation2D::~Transformation2D(){}

Transformation2D Transformation2D::get_inverse() const{
  Rotation2D inv_rot = rot_.get_inverse();
  return Transformation2D(inv_rot,-(inv_rot.rotate(trans_)));
}

Transformation2D build_Transformation2D_from_point_sets(const Vector2Ds set1,
                                                        const Vector2Ds set2) {
  IMP_assert(set1.size()==2 && set2.size()==2,
      "build_Transformation2D_from_point_sets:: The number of points "
      "in both sets must be 2");
  // v1 and v2 should be similar
  Vector2D v1 = set1[1]-set1[0];
  Vector2D v2 = set2[1]-set2[0];
  // Build the rotation to obtain vector v1
  Rotation2D R1 = build_Rotation2D_from_Vector2D(v1);
  // Build the rotation to obtain vector v2
  Rotation2D R2 = build_Rotation2D_from_Vector2D(v2);
  // Obtain the transformation from v1 to v2
  Rotation2D R = compose(R2,R1.get_inverse());
  Vector2D t = set2[0] - R.rotate(set1[0]);
  Transformation2D T(R,t);
  return T;
}

//!

IMPALGEBRA_END_NAMESPACE
