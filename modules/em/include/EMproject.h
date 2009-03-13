/**
 *  \file EMproject.h
 *  \brief Project operation for EM volumes
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
*/

#ifndef IMPEM_EMPROJECT_H
#define IMPEM_EMPROJECT_H

#include "config.h"
#include <IMP/algebra/utility.h>
#include <IMP/algebra/Matrix3D.h>
#include <IMP/algebra/Matrix2D.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/Rotation3D.h>
#include <IMP/constants.h>
#include <algorithm> // max,min


IMPEM_BEGIN_NAMESPACE


//! Projects a given 3D matrix into a 2D matrix given a direction shift
//! vector.
/**
 * The direction is used to build a rotation that ultimately describes the
 * projection plane. This rotation builds the coordinate system for the
 * projection.
 * The projection plane is the XY plane (Z=0) of this new coordinate system.
 * The additional shift vector is applied in the coordinate system for the
 * projection.
 * The projection model is:
 *               p = Rot * r + v
 * where:
 *   p - coordinates of a point in the projection coordinate system
 *   Rot - Rotation3D applied to a point r of the universal coordinate system
 *         employed for the Matrix3D
 *   r - coordinates of a point r in the of the universal coordinate system of
 *        Matrix3D
 *   v - shift vector applied to p in the coordinate system of the projection.
 *
 * \param[in] m3 A matrix3D of values to project.
 * \param[out] m2  A Matrix2D of floats to store the projection.
 * \param[in] Ydim size in rows for the Matrix2D
 *            that is to contain the projection
 * \param[in] Xdim size in columns for the Matrix2D
 *            that is to contain the projection
 * \param[in] direction vector containing the direction of projection desired
 * \param[in] shift Shift vector applied to p in the coordinate
 *            system of the projection.
 * \param[in] equality_tolerance tolerance allowed to consider a value in the
 *            direction as zero.

 * \note The function assumes that the matrices are given and stored
 *   with the (z,y,x)
 *   convention for 3D and (y,x) for 2D. But it expects and operates all the
 *   vector3D using the (x,y,z) convention.
 */
template<typename T>
void project(IMP::algebra::Matrix3D<T>& m3,
             IMP::algebra::Matrix2D<float>& m2,
             const int Ydim,const int Xdim,
             IMP::algebra::Vector3D& direction,
             const IMP::algebra::Vector3D& shift,
             const double equality_tolerance) {

// #define DEBUG
  m2.resize(Ydim, Xdim);
  // Save the origin of the matrices
  std::vector<int> orig2D(2), orig3D(3);
  orig2D[0]=m2.get_start(0);orig2D[1]=m2.get_start(1);
  orig3D[0]=m3.get_start(0);orig3D[1]=m3.get_start(1);orig3D[2]=m3.get_start(2);
  // Center the matrices (necessary for the projection algorithm)
  m2.centered_start();
  m3.centered_start();
  // For each pixel, 4 rays of projection are computed on each direction. The
  // step is going to be 1/3 from the center of the pixel in 4 directions
  double step = 1.0 / 3.0;
  // Avoids divisions by zero and allows orthogonal rays computation
  // (If any of the direction of projection's component is zero)
  for (int ii = 0;ii < 3;ii++) {
    if (direction[ii] == 0) {
      direction[ii] = equality_tolerance;
    }
  }
  IMP::algebra::Vector3D univ_coord(0,0,1);
  IMP::algebra::Vector3D vv=IMP::algebra::vector_product(univ_coord,direction);
  IMP::algebra::Rotation3D Rot;
  // If the norm of vv is near 0, the direction requested is z or -z
  if(IMP::algebra::almost_equal(vv.get_magnitude(),0.0,2*equality_tolerance)) {
    double product = univ_coord.scalar_product(direction);
    if(product < 0.0 ) {
      // The requested direction is -z
      Rot = IMP::algebra::rotation_about_axis(direction,PI);
    }
    else {
      // The requested direction is z
      Rot = IMP::algebra::rotation_about_axis(direction,0.0);
    }
  }
  else {
    // Obtain the rotation around the direction of projection
    // This quaternion allows to pass from the universal coordinate system to
    // the coordinate system of the projection
    Rot = IMP::algebra::rotation_between_two_vectors(univ_coord,direction);
  }

  // We are interested in the inverse rotation (that one that allows to pass
  // form the projection coordinate system to the universal coordinate system)
  IMP::algebra::Rotation3D InvRot = Rot.get_inverse();
#ifdef DEBUG
  std::cout << " direction " << direction << std::endl;
  std::cout << "Rotation: " << Rot << std::endl;
  std::cout << "vector_product " << vv << std::endl;
  std::cout << "Inverse rotation: " << InvRot << std::endl;
#endif
  // Precalculated variables
  IMP::algebra::Vector3D init0, end0, signs, half_signs;
  for (int i = 0;i < 3;i++) {
    init0[i] = m3.get_start(2-i); // (2-i) because we are going to work
                        // with x,y,z convention for vectors and calculations,
                        // but matrices are stored as (z,y,x)
    end0[i] = m3.get_finish(2-i); // Same convention
    signs[i] = IMP::algebra::sign(direction[i]);
    half_signs[i] = 0.5 * signs[i];
  }

  IMP::algebra::Vector3D r; // A point in the coordinate system for Matrix3D m3
  IMP::algebra::Vector3D p; // A point in the coord. system of the projection
  // build projection
  for (int j = m2.get_start(1);j <= m2.get_finish(1);j++) {
    for (int i = m2.get_start(0);i <= m2.get_finish(0);i++) {
      // 4 different rays per pixel.
      double ray_sum = 0.0;  // Line integral value
      for (int rays_per_pixel = 0; rays_per_pixel < 4; rays_per_pixel++) {
#ifdef DEBUG
         std::cout << "(" << j << "," << i << ") init ray " <<
                      rays_per_pixel << std::endl;
#endif
        // point in projection coordinate system
        switch (rays_per_pixel) {
        case 0:
          p = IMP::algebra::Vector3D(j - step, i - step, 0);
          break;
        case 1:
          p = IMP::algebra::Vector3D(j - step, i + step, 0);
          break;
        case 2:
          p = IMP::algebra::Vector3D(j + step, i - step, 0);
          break;
        case 3:
          p = IMP::algebra::Vector3D(j + step, i + step, 0);
          break;
        }

        // Express p in the universal coordinate system
        if (!shift.is_zero(equality_tolerance)) {
          p -= shift;
        }
        // Get the coordinates r in the universal system corresponding to
        // the point p
#ifdef DEBUG
        std::cout << "p: " << p << std::endl;
#endif
        r = InvRot.rotate(p);
#ifdef DEBUG
        std::cout << "r: " << r << std::endl;
#endif
        // Compute the minimum and maximum alpha for the line of the ray
        // intersecting the given volume. line = r + alpha * direction
        IMP::algebra::Vector3D v_alpha_min, v_alpha_max, v_alpha, v_diff;
        double alpha_min=-1/equality_tolerance;
        double alpha_max=1/equality_tolerance;
        for (int ii = 0;ii < 3;ii++) {
          v_alpha_min[ii] = (init0[ii] - 0.5 - r[ii]) / direction[ii];
          v_alpha_max[ii] = ( end0[ii] + 0.5 - r[ii]) / direction[ii];
          // Consider this index if the component of the direction
          // is not almost zero
          if(std::abs(direction[ii]) > equality_tolerance ) {
            alpha_min=std::max(alpha_min,
                      std::min(v_alpha_min[ii],v_alpha_max[ii]));
            alpha_max=std::min(alpha_max,
                      std::max(v_alpha_min[ii],v_alpha_max[ii]));
          }
        }
#ifdef DEBUG
        std::cout << "v_alpha_min " << v_alpha_min;
        std::cout << " v_alpha_max " << v_alpha_max << std::endl;
#endif
        if (IMP::algebra::almost_equal(
           alpha_max, alpha_min, equality_tolerance)) {
#ifdef DEBUG
         std::cout << " ray skipped (" << j << "," << i << ") init ray " <<
                      rays_per_pixel << std::endl;
#endif
          continue;
        }
        // v is going to be the first voxel in the volume intersecting the ray
        IMP::algebra::Vector3D v;
        std::vector<int> idx(3);
        v = r + alpha_min * direction; // vector operation
#ifdef DEBUG
        std::cout << " v " << v << std::endl;
#endif
        // Index of the first voxel
        for (int ii=0;ii < 3;ii++) {
          idx[ii] = IMP::algebra::constrain(round(v[ii]), init0[ii], end0[ii]);
#ifdef DEBUG
          std::cout << " idx[" << ii << "]= " << idx[ii];
#endif
        }
#ifdef DEBUG
        std::cout << std::endl;
#endif
        // Follow the ray
        double alpha = alpha_min;
#ifdef DEBUG
        std::cout << " alpha_min " << alpha_min;
        std::cout << " alpha_max " << alpha_max;
        std::cout << " initial alpha " << alpha << std::endl;
#endif
        do {
          for (int ii=0;ii < 3;ii++) {
            v_alpha[ii] = (idx[ii] + half_signs[ii] - r[ii]) / direction[ii];
            v_diff[ii] = std::abs(alpha - v_alpha[ii]);
          }
          // Determine the dimension the ray will move in the next step.
          // (Is the mininum value in v_diff)
#ifdef DEBUG
          std::cout << " v_alpha " << v_alpha << std::endl;
          std::cout << " v_diff " << v_diff << std::endl;
#endif
          double diff_alpha=std::min(std::min(v_diff[0],v_diff[1]),v_diff[2]);

//          std::cout << " diff_alpha " << diff_alpha << std::endl;
          // It is supposed that the first index of the Matrix3D is for Z,
          // that's why the indices are inverted
#ifdef DEBUG
          std::cout << "inverted idx " <<
              idx[2] << " " << idx[1] << " " << idx[0] << " | ";
          std::cout << "m3(inverted idx) = " <<
              m3(idx[2], idx[1], idx[0]) << std::endl;
#endif
          ray_sum += diff_alpha * m3(idx[2], idx[1], idx[0]);
          // update the indexes in the required dimensions
          for (int ii=0;ii < 3;ii++) {
            if (IMP::algebra::almost_equal(diff_alpha, v_diff[ii],
                equality_tolerance)) {
              alpha = v_alpha[ii];
              idx[ii] += signs[ii];
            }
          }
#ifdef DEBUG
          std::cout << " alpha =" << alpha << std::endl;
#endif
        } while ((alpha_max - alpha) > equality_tolerance); // end of the ray
#ifdef DEBUG
        std::cout << " final alpha =" << alpha << std::endl;
        std::cout << " ray_sum =" << ray_sum << std::endl;
#endif
      } // for involving the 4 rays
      // Average the value of the 4 rays
      m2(i,j) = ray_sum * 0.25;
#ifdef DEBUG
       std::cout << "m2(" << i << "," << j << ")=  " << m2(i,j) << std::endl;
#endif
    } // i for
  } // j for

  // Reindex the matrices again
  m2.reindex(orig2D);
  m3.reindex(orig3D);
}

IMPEM_END_NAMESPACE

#endif  /* IMPEM_EMPROJECT_H */
