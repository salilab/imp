/**
 *  \file IMP/algebra/Vector3D.h   \brief Simple 3D vector class.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_VECTOR_3D_H
#define IMPALGEBRA_VECTOR_3D_H

#include <IMP/types.h>
#include <IMP/base_macros.h>
#include <IMP/exception.h>

#include <numeric>
#include <cmath>

#include "VectorD.h"

IMPALGEBRA_BEGIN_NAMESPACE

/** \name 3D Vectors
    We provide a specialization of VectorD for 3-space and
    several additional functions on it.
    @{
*/

//! Return the vector product (cross product) of two vectors.
/** \see Vector3D
 */
inline Vector3D get_vector_product(const Vector3D &p1, const Vector3D &p2) {
  return Vector3D(p1[1] * p2[2] - p1[2] * p2[1], p1[2] * p2[0] - p1[0] * p2[2],
                  p1[0] * p2[1] - p1[1] * p2[0]);
}

/* //! Get angle between vectors p1 and p2 (with appropriate sign depending on where */
/* //! p2 is in relation to p1) */
/* inline Vector3D get_angle(const Vector3D &p1, const Vector3D &p2) { */
/*   // see wikipedia on great circles - atan formula is numerically more stable */
/*   // than using acos or asin. */
/*   double sin_sigma = get_vector_product(p1,p2).get_magnitude(); */
/*   double cos_sigma = p1*p2; */
/*   double sigma = atan2(sin_sigma,cos_sigma); */
/*   return sigma; */
/* } */

//! Return a vector that is perpendicular to the given vector
/** \note This is occasionally referred to in the code as a "vertical" vector.
    \see Vector3D
*/
inline Vector3D get_orthogonal_vector(const Vector3D &v) {
  unsigned int maxi = 0;
  if (std::abs(v[1]) > std::abs(v[0])) maxi = 1;
  if (std::abs(v[2]) > std::abs(v[maxi])) maxi = 2;
  if (std::abs(v[maxi]) < .0001) {
    return Vector3D(0.0, 0.0, 0.0);
  } else {
    Vector3D ret = get_ones_vector_d<3>();
    ret[maxi] = (-v[(maxi + 1) % 3] - v[(maxi + 2) % 3]) / v[maxi];
    IMP_INTERNAL_CHECK(ret * v < .0001, "Vectors are not perpendicular");
    return ret;
  }
}

//! Return the centroid of a set of vectors
/** \see Vector3D
 */
inline Vector3D get_centroid(const Vector3Ds &ps) {
  return std::accumulate(ps.begin(), ps.end(), get_zero_vector_d<3>()) /
         ps.size();
}

//! Return the radius of gyration of a set of points
/**
   \see IMP::atom::get_radius_of_gyration()
 */
inline double get_radius_of_gyration(const Vector3Ds &ps) {
  algebra::Vector3D centroid = get_centroid(ps);
  double rg = 0;
  for (unsigned int i = 0; i < ps.size(); i++) {
    rg += get_squared_distance(ps[i], centroid);
  }
  rg /= ps.size();
  return sqrt(rg);
}

/** @} */

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_VECTOR_3D_H */
