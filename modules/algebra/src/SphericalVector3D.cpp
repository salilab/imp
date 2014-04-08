/**
 *  \file  SphericalVector3D.cpp
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
*/
#include "IMP/algebra/SphericalVector3D.h"

IMPALGEBRA_BEGIN_NAMESPACE

Vector3D SphericalVector3D::get_cartesian_coordinates() {
  return Vector3D(v_[0] * cos(v_[2]) * sin(v_[1]),
                  v_[0] * sin(v_[2]) * sin(v_[1]), v_[0] * cos(v_[1]));
}

void SphericalVector3D::set_cartesian_coordinates(const Vector3D& v) {
  v_[0] = v.get_magnitude();
  v_[1] = atan2(sqrt(v[0] * v[0] + v[1] * v[1]), v[2]);
  v_[2] = atan2(v[1], v[0]);
}

bool SphericalVector3D::check(double r, double theta, double psi) {
  if (r < 0) {
    return false;
  }
  if (theta < -PI || theta > PI) {
    return false;
  }
  if (psi < -2 * PI || psi > 2 * PI) {
    return false;
  }
  return true;
}

IMPALGEBRA_END_NAMESPACE
