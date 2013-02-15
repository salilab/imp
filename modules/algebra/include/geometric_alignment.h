/**
 *  \file IMP/algebra/geometric_alignment.h
 *  \brief align sets of points.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPALGEBRA_GEOMETRIC_ALIGNMENT_H
#define IMPALGEBRA_GEOMETRIC_ALIGNMENT_H

#include <IMP/algebra/algebra_config.h>
#include "Vector3D.h"
#include "SphereD.h"
#include "Rotation3D.h"
#include "Transformation3D.h"
#include "Transformation2D.h"
#include "internal/tnt_array2d.h"
#include "internal/jama_svd.h"
#include <IMP/base/check_macros.h>
#include <IMP/base/log.h>


IMPALGEBRA_BEGIN_NAMESPACE


//! Compute the rigid transform bringing the first point set to the second
/** The points are assumed to be corresponding (that is, from[0] is aligned
    to to[0] etc.). The alignment computed is that which minimized the
    sum of squared distances between corresponding points. Return the
    \f[ \textit{argmin}_T
    \sum \left|T\left(f\left[i\right]\right)-t[i]\right|^2 \f]

    If the point sets lie in a 1 or 2 dimensional subspace,
    the alignment algorithm
    is unstable and not guaranteed to work. A warning is printed in this
    case.

    \genericgeometry

    \relatesalso Transformation3D
    \see Vector3D
 */
template <class Vector3DsOrXYZs0,
          class Vector3DsOrXYZs1>
inline IMP::algebra::Transformation3D
get_transformation_aligning_first_to_second(
const Vector3DsOrXYZs0 &source,
const Vector3DsOrXYZs1 &target
) {
  IMP_INTERNAL_CHECK(source.size() == target.size(), "sizes don't match");
  IMP_INTERNAL_CHECK(source.size() >0, "Points are needed");
  // compute the centroid of the points and transform
  // pointsets so that their centroids coinside

  Vector3D center_source(0,0,0), center_target(0,0,0);
  for (unsigned int i=0; i< source.size(); ++i) {
    //double x= p_it->x();
    center_source+= get_vector_d_geometry(source[i]);
    center_target += get_vector_d_geometry(target[i]);
    IMP_LOG_VERBOSE( i << ": (" << get_vector_d_geometry(source[i])
            << ") (" << get_vector_d_geometry(target[i]) << ")\n");
  }
  center_source = center_source/source.size();
  center_target = center_target/target.size();

  IMP_LOG_VERBOSE( "Centers are (" << center_source << ") (" << center_target
          << ")\n");
  Vector3Ds shifted_source(source.size()), shifted_target(target.size());
  for (unsigned int i=0; i< source.size(); ++i) {
    shifted_source[i]=get_vector_d_geometry(source[i])-center_source;
    shifted_target[i]= get_vector_d_geometry(target[i])-center_target;
  }

  // covariance matrix
  internal::TNT::Array2D<double> H(3, 3);
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      H[i][j] = 0;
    }
  }
  for (unsigned int i = 0; i < source.size(); i++) {
    for (int j = 0; j < 3; j++) {
      for (int k = 0; k < 3; k++) {
        H[j][k] += shifted_source[i][j]*shifted_target[i][k];
      }
    }
  }

  IMP_LOG_VERBOSE( "H is " << H << std::endl);

  internal::JAMA::SVD<double> svd(H);
  internal::TNT::Array2D<double> U(3, 3), V(3, 3);
  svd.getU(U);
  svd.getV(V);

  IMP_LOG_VERBOSE( "SVD is " << U << std::endl << V << std::endl);

  internal::TNT::Array1D<double> SV;
  svd.getSingularValues(SV);
  double det= SV[0]*SV[1]*SV[2];
  IMP_IF_CHECK(base::USAGE) {
    if (det < .00001) {
      IMP_LOG_TERSE( "SOURCE:\n");
      for (unsigned int i=0; i< source.size(); ++i) {
        IMP_LOG_TERSE( source[i] << std::endl);
      }
      IMP_LOG_TERSE( "TO:\n");
      for (unsigned int i=0; i< source.size(); ++i) {
        IMP_LOG_TERSE( target[i] << std::endl);
      }
      IMP_LOG_TERSE( H);
      IMP_WARN("Degenerate point set. I may not be able to align them."
               << std::endl);
    }
  }

  IMP_IF_LOG(VERBOSE) {
    internal::TNT::Array2D<double> Sigma(3,3, 0.0);

    for (int i=0; i < 3; ++i) {
      Sigma[i][i]= SV[i];
    }

    IMP_LOG_VERBOSE( "Reconstructed is "
            << internal::TNT::matmult(internal::TNT::matmult(U,Sigma),
                                      internal::TNT::transpose(V))
            << std::endl);
  }

  // the rotation matrix is R = VU^T
  internal::TNT::Array2D<double> UT = internal::TNT::transpose(U);
  internal::TNT::Array2D<double> rot(3, 3);
  rot = matmult(V, UT);

  // check for reflection
  if (determinant(rot) < 0) {
    IMP_LOG_VERBOSE( "Flipping matrix"<<std::endl);
    internal::TNT::Array2D<double> VT = internal::TNT::transpose(V);
    internal::TNT::Array2D<double> UVT = internal::TNT::matmult(U, VT);
    internal::TNT::Array2D<double> S(3, 3);
    S[0][0] = S[1][1] = 1;
    S[2][2] = determinant(UVT);
    S[0][1] = S[0][2] = S[1][0] = S[1][2] = S[2][0] = S[2][1] = 0;
    rot = internal::TNT::matmult(internal::TNT::matmult(U, S), VT);
  }

  IMP_LOG_VERBOSE( "Rotation matrix is " << rot << std::endl);

  Rotation3D rotation
    = get_rotation_from_matrix(rot[0][0], rot[0][1], rot[0][2],
                               rot[1][0], rot[1][1], rot[1][2],
                               rot[2][0], rot[2][1], rot[2][2]);
  IMP_LOG_VERBOSE( "Rotation is " << rotation << std::endl);
  Vector3D translation=center_target - rotation.get_rotated(center_source);

  Transformation3D ret(rotation, translation);
  return ret;
}



//! Builds the transformation required to obtain a set of points from
//! the first one
/**
  \note The function assumes that the relative distances between points
  are conserved.
**/
IMPALGEBRAEXPORT Transformation2D get_transformation_aligning_pair(
                       const Vector2Ds& set_from,
                       const Vector2Ds& set_to);

// implemented in Transformation2D

IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_GEOMETRIC_ALIGNMENT_H */
