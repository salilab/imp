/**
 *  \file IMP/algebra/distance.h
 *  \brief distance metrics
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_DISTANCE_H
#define IMPALGEBRA_DISTANCE_H

#include <IMP/algebra/algebra_config.h>
#include "Transformation3D.h"
#include "VectorD.h"
#include "IMP/base_types.h"

IMPALGEBRA_BEGIN_NAMESPACE

//! Calculate the root mean square deviation between two sets of 3D points.
/**
   \note the function assumes correspondence between the two sets of
   points and does not perform rigid alignment.

   \genericgeometry
 */
template <class Vector3DsOrXYZs0, class Vector3DsOrXYZs1>
inline double get_rmsd_transforming_first(const Transformation3D& tr,
                                          const Vector3DsOrXYZs0& m1,
                                          const Vector3DsOrXYZs1& m2) {
  IMP_USAGE_CHECK(std::distance(m1.begin(), m1.end()) ==
                  std::distance(m2.begin(), m2.end()),
                  "The input sets of XYZ points "
                  << "should be of the same size");
  double rmsd = 0.0;
  typename Vector3DsOrXYZs0::const_iterator it0 = m1.begin();
  typename Vector3DsOrXYZs1::const_iterator it1 = m2.begin();
  for (; it0 != m1.end(); ++it0, ++it1) {
    Vector3D tred =
      tr.get_transformed(get_vector_d_geometry(*it0));
    rmsd += get_squared_distance(tred, get_vector_d_geometry(*it1));
  }
  return std::sqrt(rmsd / m1.size());
}

/** Se get_rmsd_transforming_first(). */
template <class Vector3DsOrXYZs0, class Vector3DsOrXYZs1>
inline double get_rmsd(const Vector3DsOrXYZs0& m1, const Vector3DsOrXYZs1& m2) {
  return get_rmsd_transforming_first(get_identity_transformation_3d(), m1, m2);
}

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_DISTANCE_H */
