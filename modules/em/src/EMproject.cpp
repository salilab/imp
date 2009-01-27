/**
 *  \file EMproject.cpp
 *  \brief Projection operation for 3D volumes
 *  \author Javier Velazquez-Muriel
 *  Copyright 2007-8 Sali Lab. All rights reserved.
*/

#include <IMP/em/EMproject.h>

IMPEM_BEGIN_NAMESPACE

template<typename T>
void project(algebra::Matrix3D<T>& m3, algebra::Matrix2D<Float>& m2,
             Int Ydim, Int Xdim, Float Rot, Float Tilt, Float Psi,
             algebra::Vector3D shift, equality_tolerance = 1e-6)
{
  // Initialise Matrix2D
  m2.resize(Ydim, Xdim);

  // For each pixel, 4 rays of proyection are computed on each direction. The
  // step is going to be 1/3 of the center of the pixel in 4 directions
  Float step = 1.0 / 3.0;

  // Build the rotation matrix. Compatible with the coordinate system employed
  // in SPIDER and Xmipp.
  EulerMatrixZYZ RotMat(-Rot, Tilt, -Psi);

  // Direction perpendicular to the projection plane (last row of the rotation
  // matrix)
  algebra::Vector3D direction(RotMat[2][0], RotMat[2][1], RotMat[2][2])

  // Avoids divisions by zero and allows orthogonal rays computation
  for (Int ii = 0;ii < 3;ii++) {
    if (direction[ii] == 0) {
      direction[ii] = equality_tolerance;
    }
  }

  // Precalculated variables
  algebra::Vector3D init0, end0, signs, half_signs;
  for (Int i = 0;i < 3;i++) {
    init0[i] = m3.get_start(i);
    end0[i] = m3.get_finish(i);
    signs[i] = sgn(direction[i]);
    half_signs[i] = 0.5 * signs[i];
  }

  algebra::Vector3D r; // Coordinates of a point in the
  //coordinate system for Matrix3D m3
  algebra::Vector3D p; // Coordinates of a point in the
  // projection coordinate system

  for (Int j = 0;j < m2.get_rows();j++) {
    for (Int i = 0;i < m2.get_columns();i++) {

      Float ray_sum = 0.0;  // Line Integral value

      // 4 different rays per pixel.
      for (Int rays_per_pixel = 0; rays_per_pixel < 4; rays_per_pixel++) {
        // point in projection coordinate system
        switch (rays_per_pixel) {
        case 0:
          p = algebra::Vector3D(i - step, j - step, 0);
          break;
        case 1:
          p = algebra::Vector3D(i - step, j + step, 0);
          break;
        case 2:
          p = algebra::Vector3D(i + step, j - step, 0);
          break;
        case 3:
          p = algebra::Vector3D(i + step, j + step, 0);
          break;
        }

        // Express p in the universal coordinate system
        if (shift != NULL) {
          p -= shift;
        }
        r = RotMat.rotate(p); // Apply the rotation to p

        // Compute the minimum and maximum alpha for the ray
        // Intersecting the given volume
        // Alpha is the number of voxels to advance on each direction
        algebra::vector3D v_alpha_min, v_alpha_max, v_alpha, v_diff;
        for (Int ii = 0;ii < 3;ii++) {
          v_alpha_min[ii] = (init0[ii] - 0.5 - r[ii]) / direction[ii];
          v_alpha_max[ii] = (end0[ii] + 0.5 - r[ii]) / direction[ii];
        }

        Float alpha_min = std::max(std::min(v_alpha_min[0], v_alpha_max[0]),
                                   std::min(v_alpha_min[1], v_alpha_max[1]));
        alpha_min  = std::max(alpha_min,
                              std::min(v_alpha_min[2], v_alpha_max[2]));
        Float alpha_max = std::min(std::max(v_alpha_min[0], v_alpha_max[0]),
                                   std::max(v_alpha_min[1], v_alpha_max[1]));
        alpha_max       = std::min(alpha_max,
                                   std::max(v_alpha_min[2], v_alpha_max[2]));

        if (equal(alpha_max, alpha_min, equalitiy_tolerance)) {
          continue;
        }

#ifdef DEBUG
        std::cout << "Pixel p:  " << p << std::endl
        << "Univ:   " << r << std::endl
        << "Dir:  " << direction << std::endl
        << "Alpha min " << v_alpha_min << << std::endl
        << "Alpha max " << v_alpha_max << << std::endl
        << "alpha  :" << alpha_min  << " " << alpha_max  << std::endl
        << std::endl;
#endif

        // v is going to be the first voxel in the volume intersecting the ray
        algebra::Vector3D v;
        std::vector<Int> idx(3);
        v = r + alpha_min * direction;
        // Index of the first voxel
        for (Int ii;ii < 3;ii++) {
          idx[ii] = threshold(round(v[ii]), init0[ii], end0[ii]);
        }
#ifdef DEBUG
        std::cout << "First voxel: " << v << std::endl;
        std::cout << "   First index: " << idx << std::endl;
        std::cout << "   Alpha_min: " << alpha_min << std::endl;
#endif
        // Follow the ray
        Float alpha = alpha_min;

        do {
#ifdef DEBUG
          std::cout << " Current value volume(z,y,x): " << m3[idx[2]] <<
          " " << m3[idx[1]] << " " << m3[idx[0]] << std::endl;
#endif


          for (Int ii;ii < 3;ii++) {
            v_alpha[ii] = idx[ii] + (half_signs[ii] - r[ii]) / direction[ii];
            v_diff[ii] = alpha - v_alpha[ii];
          }
          // Determine the dimension the ray will move in the next step.
          Float diff_alpha=std::min(std::min(v_diff[0],v_diff[1]),v_diff[2]);

          // It is supposed that the first index of the matrix3D is for Z,
          // that's why the indices are inverted
          ray_sum += diff_alpha * m3(idx[2], idx[1], idx[0]);


          for (Int ii;ii < 3;ii++) {
            if (equal(diff_alpha, v_diff[ii], equality_tolerance)) {
              alpha = v_alpha[ii];
              idx[ii] += signs[ii];
            }
          }

#ifdef DEBUG
          std::cout << "v_alpha: " << v_alpha << " alpha "
          << alpha << std::endl;
          v += (diff_alpha * direction);
          std::cout << "  Next point of the ray: " << v << std::endl
          << "  Index: " << idx << std::endl
          << "  diff_alpha: " << diff_alpha << std::endl
          << "  ray_sum: " << ray_sum << std::endl
          << "  Alfa tot: " << alpha << "alpha_max: " << alpha_max <<
          std::endl;
#endif
        } while ((alpha_max - alpha) > equality_tolerance);
      } // for involving the 4 rays
      // Average the value of the 4 rays
      m2(i, j) = ray_sum * 0.25;
#ifdef DEBUG
      std::cout << "Assigned m2(" << i << "," << j << ")= "
      << m2(i, j) << std::endl;
#endif
    } // i for
  } // j for
}


IMPEM_END_NAMESPACE
