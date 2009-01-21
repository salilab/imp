/**
 *  \file geometric_alignment.cpp
 *  \brief Support for rigid bodies.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/algebra/geometric_alignment.h"
#include <IMP/algebra/internal/tnt_array2d.h>
#include <IMP/algebra/internal/jama_svd.h>

IMPALGEBRA_BEGIN_NAMESPACE


Transformation3D
rigid_align_first_to_second(const std::vector<Vector3D> &from,
                            const std::vector<Vector3D> &to) {
  IMP_assert(from.size() == to.size(), "sizes don't match");
  // compute the centroid of the points and transform
  // pointsets so that their centroids coinside

  Vector3D center_from(0,0,0), center_to(0,0,0);
  for (unsigned int i=0; i< from.size(); ++i) {
    //double x= p_it->x();
    center_from+= from[i];
    center_to += to[i];
  }
  center_from = center_from/from.size();
  center_to = center_to/to.size();

  std::vector<Vector3D> shifted_from(from.size()), shifted_to(to.size());
  for (unsigned int i=0; i< from.size(); ++i) {
    shifted_from[i]= from[i]-center_from;
    shifted_to[i]= to[i]-center_to;
  }

  // covariance matrix
  internal::TNT::Array2D<double> H(3, 3);
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      H[i][j] = 0;
    }
  }
  for (unsigned int i = 0; i < from.size(); i++) {
    for (int j = 0; j < 3; j++) {
      for (int k = 0; k < 3; k++) {
        H[j][k] += shifted_from[i][j]*shifted_to[i][k];
      }
    }
  }

  internal::JAMA::SVD<double> svd(H);
  internal::TNT::Array2D<double> U(3, 3), V(3, 3);
  svd.getU(U);
  svd.getV(V);

  // the rotation matrix is R = VU^T
  internal::TNT::Array2D<double> UT = internal::TNT::transpose(U);
  internal::TNT::Array2D<double> rot(3, 3);
  rot = matmult(V, UT);

  // check for reflection
  if (determinant(rot) < 0) {
    internal::TNT::Array2D<double> VT = internal::TNT::transpose(V);
    internal::TNT::Array2D<double> UVT = internal::TNT::matmult(U, VT);
    internal::TNT::Array2D<double> S(3, 3);
    S[0][0] = S[1][1] = 1;
    S[2][2] = determinant(UVT);
    S[0][1] = S[0][2] = S[1][0] = S[1][2] = S[2][0] = S[2][1] = 0;
    rot = internal::TNT::matmult(internal::TNT::matmult(U, S), VT);
  }

  Rotation3D rotation
    = rotation_from_matrix(rot[0][0], rot[0][1], rot[0][2],
                           rot[1][0], rot[1][1], rot[1][2],
                           rot[2][0], rot[2][1], rot[2][2]);
  Vector3D translation=center_to - rotation.rotate(center_from);

  return Transformation3D(rotation, translation);
}


IMPALGEBRA_END_NAMESPACE
