/**
 *  \file Vector3D.h   \brief Simple 3D vector class.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_VECTOR_3D_H
#define IMPALGEBRA_VECTOR_3D_H

#include <IMP/base_types.h>
#include <IMP/macros.h>
#include <IMP/exception.h>

#include <numeric>
#include <cmath>

#include "VectorD.h"

IMPALGEBRA_BEGIN_NAMESPACE

IMPALGEBRA_EXPORT_TEMPLATE(VectorD<3>);

/** \name 3D Vectors
    We provide a specialization of VectorD for 3-space and
    several additional functions on it.
    @{
*/

//! Returns the vector product (cross product) of two vectors.
/** \relatesalso VectorD<3>
 */
inline VectorD<3> get_vector_product(const VectorD<3>& p1,
                                     const VectorD<3>& p2) {
  return VectorD<3>(p1[1]*p2[2]-p1[2]*p2[1],
                  p1[2]*p2[0]-p1[0]*p2[2],
                  p1[0]*p2[1]-p1[1]*p2[0]);
}
//! Return a vector that is perpendicular to the given vector
/** Or, if you are Israeli, it is a vertical vector.
    \relatesalso VectorD<3>
*/
inline VectorD<3> get_orthogonal_vector(const VectorD<3> &v) {
  if (v[0] != 0) {
    return VectorD<3>((-v[1]-v[2])/v[0],1,1);
  } else if (v[1] != 0.0) {
    return VectorD<3>(1,(-v[0]-v[2])/v[1],1);
  } else if (v[2] != 0.0) {
    return VectorD<3>(1,1,(-v[0]-v[1])/v[2]);
  } else {
    return VectorD<3>(0.0,0.0,0.0);
  }
}

//! Returns the centroid of a set of vectors
/** \relatesalso VectorD<3>
 */
inline VectorD<3> get_centroid(const std::vector<VectorD<3> > &ps) {
  return std::accumulate(ps.begin(), ps.end(),
                         get_zero_vector_d<3>())/ps.size();
}

/** @} */
/** \genericgeometry */
inline const VectorD<3> &get_geometry(const VectorD<3> &v) {
  return v;
}

IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_VECTOR_3D_H */
