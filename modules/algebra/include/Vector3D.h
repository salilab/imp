/**
 *  \file Vector3D.h   \brief Simple 3D vector class.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_VECTOR_3D_H
#define IMPALGEBRA_VECTOR_3D_H

#include <IMP/base_types.h>
#include <IMP/macros.h>
#include <IMP/exception.h>

#include <cmath>

#include "VectorD.h"

IMPALGEBRA_BEGIN_NAMESPACE

//! A location in 3D
typedef VectorD<3> Vector3D;

//! A collection of Vector3D
typedef std::vector<Vector3D> Vector3Ds;

//! Returns the vector product (cross product) of two vectors.
/** \relates Vector3D
 */
inline Vector3D vector_product(const Vector3D& p1, const Vector3D& p2) {
  return Vector3D(p1[1]*p2[2]-p1[2]*p2[1],
                  p1[2]*p2[0]-p1[0]*p2[2],
                  p1[0]*p2[1]-p1[1]*p2[0]);
}
//! Return a vector that is perpendicular to the given vector
/** Or, if you are Israeli, it is a vertical vector.
    \relates Vector3D
*/
inline Vector3D orthogonal_vector(const Vector3D &v) {
  if (v[0] != 0) {
    return Vector3D((-v[1]-v[2])/v[0],1,1);
  } else if (v[1] != 0.0) {
    return Vector3D(1,(-v[0]-v[2])/v[1],1);
  } else if (v[2] != 0.0) {
    return Vector3D(1,1,(-v[0]-v[1])/v[2]);
  } else {
    return Vector3D(0.0,0.0,0.0);
  }
}

//! Converts cartesian coordinates to spherical ones
/**
  The order returned is radial distance, zenith, and azimuth (r, tetha, psi)
*/
inline Vector3D cartesian_to_spherical(const Vector3D &v) {
  double r =   v.get_magnitude();
  return Vector3D(r, atan2(v[1],v[0]),
                  atan2(std::sqrt(v[0]*v[0]+v[1]*v[1]),v[2]));
}

//! Converts spherical coordinates to cartesian ones
/**
  The order assumed for the vector of spherical coordinates  is radial
distance, zenith, and azimuth (r, tetha, psi)
*/
inline Vector3D spherical_to_cartesian(const Vector3D &v) {
  return Vector3D(v[0]*cos(v[2])*sin(v[1]),
                  v[0]*sin(v[2])*sin(v[1]),
                  v[0]*cos(v[1]));
}


IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_VECTOR_3D_H */
