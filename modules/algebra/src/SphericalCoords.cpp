/**
 *  \file  SphericalCoords.cpp
 *  \author Javier Velazquez-Muriel
 *  Copyright 2007-9 Sali Lab. All rights reserved.
*/
#include "IMP/algebra/SphericalCoords.h"

IMPALGEBRA_BEGIN_NAMESPACE

Vector3D SphericalCoords::to_cartesian() {
  return Vector3D(_v[0]*cos(_v[2])*sin(_v[1]),
                  _v[0]*sin(_v[2])*sin(_v[1]),
                  _v[0]*cos(_v[1]));
};

void SphericalCoords::from_cartesian(Vector3D& v) {
  double r =   v.get_magnitude();
  _v[0] = r;
  _v[1] = atan2(v[1],v[0]);
  _v[2] = atan2(sqrt(v[0]*v[0]+v[1]*v[1]),v[2]);
};

IMPALGEBRA_END_NAMESPACE
