/**
 *  \file Vector3D.h   \brief Simple 3D vector class.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
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

/* Microsoft compilers require that if a class is exported from a DLL,
   all member objects are too. Thus, we make sure that we export the
   Vector3Ds class here; otherwise, both the statistics and display modules
   will attempt to export Vector3Ds (since they both contain exported classes
   that have Vector3Ds members) and then any module that attempts to link
   against both statistics and display (e.g. domino2) will fail to link with
   LNK2005/LNK1169 errors (multiply defined symbols). See also
   http://support.microsoft.com/kb/q168958/

   Note that STL members other than std::vector may be impossible to export
   on Windows systems, due to hidden nested classes.
 */
IMPALGEBRA_EXPORT_TEMPLATE(std::allocator<VectorD<3> >);
IMPALGEBRA_EXPORT_TEMPLATE(std::vector<VectorD<3> >);

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
  unsigned int maxi=0;
  if (std::abs(v[1]) > std::abs(v[0])) maxi=1;
  if (std::abs(v[2]) > std::abs(v[maxi])) maxi=2;
  if (std::abs(v[maxi]) < .0001) {
    return VectorD<3>(0.0,0.0,0.0);
  } else {
    VectorD<3> ret= get_ones_vector_d<3>();
    ret[maxi]=(-v[(maxi+1)%3]-v[(maxi+2)%3])/v[maxi];
    IMP_INTERNAL_CHECK(ret*v < .0001, "Vectors are not perpendicular");
    return ret;
  }
}

//! Returns the centroid of a set of vectors
/** \relatesalso VectorD<3>
 */
inline VectorD<3> get_centroid(const std::vector<VectorD<3> > &ps) {
  return std::accumulate(ps.begin(), ps.end(),
                         get_zero_vector_d<3>())/ps.size();
}

//! Return the radius of gyration of a set of points
/**
   \see IMP::atom::get_radius_of_gyration()
 */
inline double get_radius_of_gyration(const std::vector<VectorD<3> > &ps) {
  algebra::Vector3D centroid= get_centroid(ps);
  double rg = 0;
  for (unsigned int i = 0; i < ps.size(); i++) {
    rg += get_squared_distance(ps[i], centroid);
  }
  rg /= ps.size();
  return sqrt(rg);
}

/** @} */

IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_VECTOR_3D_H */
