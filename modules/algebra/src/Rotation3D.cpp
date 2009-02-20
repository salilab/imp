/**
 *  \file Rotation3D.cpp   \brief Simple 3D rotation class.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */
#include "IMP/algebra/Rotation3D.h"
#include "IMP/algebra/VectorD.h"
#include "IMP/algebra/vector_generators.h"

IMPALGEBRA_BEGIN_NAMESPACE

Rotation3D::~Rotation3D() {
}

Rotation3D Rotation3D::get_inverse() const {
  IMP_check(v_.get_squared_magnitude() != 0,
            "Attempting to invert uninitialized rotation",
            InvalidStateException);
  Rotation3D ret(v_[0], -v_[1], -v_[2], -v_[3]);
  return ret;
}



Rotation3D rotation_from_matrix(double m11,double m12,double m13,
                                double m21,double m22,double m23,
                                double m31,double m32,double m33) {
  IMP_IF_CHECK(EXPENSIVE) {
    Vector3D v0(m11, m12, m13);
    Vector3D v1(m21, m22, m23);
    Vector3D v2(m31, m32, m33);
    IMP_check(std::abs(v0.get_squared_magnitude()-1) < .1,
              "The passed matrix is not a rotation matrix (row 0).",
              ValueException);
    IMP_check(std::abs(v1.get_squared_magnitude()-1) < .1,
              "The passed matrix is not a rotation matrix (row 1).",
              ValueException);
    IMP_check(std::abs(v2.get_squared_magnitude()-1) < .1,
              "The passed matrix is not a rotation matrix (row 2).",
              ValueException);
    IMP_check(std::abs(v0 *v1) < .1,
              "The passed matrix is not a rotation matrix (row 0, row 1).",
              ValueException);
    IMP_check(std::abs(v0 *v2) < .1,
              "The passed matrix is not a rotation matrix (row 0, row 2).",
              ValueException);
    IMP_check(std::abs(v1 *v2) < .1,
              "The passed matrix is not a rotation matrix (row 1, row 2).",
              ValueException);
    Vector3D c0(m11, m21, m31);
    Vector3D c1(m12, m22, m32);
    Vector3D c2(m13, m23, m33);
    IMP_check(std::abs(c0.get_squared_magnitude()-1) < .1,
              "The passed matrix is not a rotation matrix (col 0).",
              ValueException);
    IMP_check(std::abs(c1.get_squared_magnitude()-1) < .1,
              "The passed matrix is not a rotation matrix (col 1).",
              ValueException);
    IMP_check(std::abs(c2.get_squared_magnitude()-1) < .1,
              "The passed matrix is not a rotation matrix (col 2).",
              ValueException);
    IMP_check(std::abs(c0 *c1) < .1,
              "The passed matrix is not a rotation matrix (col 0, col 1).",
              ValueException);
    IMP_check(std::abs(c0 *c2) < .1,
              "The passed matrix is not a rotation matrix (col 0, col 2).",
              ValueException);
    IMP_check(std::abs(c1 *c2) < .1,
              "The passed matrix is not a rotation matrix (col 1, col 2).",
              ValueException);
  }
  double a,b,c,d;
  a = fabs(1+m11+m22+m33)/4;
  b = fabs(1+m11-m22-m33)/4;
  c = fabs(1-m11+m22-m33)/4;
  d = fabs(1-m11-m22+m33)/4;

  // make sure quat is normalized.
  double sum = a+b+c+d;
  a = sqrt(a/sum);
  b = sqrt(b/sum);
  c = sqrt(c/sum);
  d = sqrt(d/sum);

  if (m32-m23 < 0.0) b=-b;
  if (m13-m31 < 0.0) c=-c;
  if (m21-m12 < 0.0) d=-d;
  return Rotation3D(a,b,c,d);
}

Rotation3D random_rotation() {
  VectorD<4> r= random_vector_on_unit_sphere<4>();
  return Rotation3D(r[0], r[1], r[2], r[3]);
}
IMPALGEBRA_END_NAMESPACE
