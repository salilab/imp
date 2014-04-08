/**
 *  \file rotation_helper.h
 *  \brief Funtions related with rotations in em2d
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_INTERNAL_ROTATION_HELPER_H
#define IMPEM2D_INTERNAL_ROTATION_HELPER_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/em2d/opencv_interface.h"
#include "IMP/algebra/algebra_config.h"
#include "IMP/algebra/SphericalVector3D.h"
#include "IMP/algebra/Vector3D.h"
#include "IMP/algebra/Rotation3D.h"

IMPEM2D_BEGIN_INTERNAL_NAMESPACE

//! Distributes N points into a semisphere following an spiral from the
//! pole. Zenith = [0,PI/2) and azimuth = [0,2PI)
/**
   \param[in] N number of points
   \param[out] vs A vector of SphericalVector3D classes
   \param[in] r radius of the sphere
   \note Saff, The Mathematical Intelligencer (1997)
**/
void semispherical_spiral_distribution(const unsigned long N,
                                       algebra::SphericalVector3Ds &vs,
                                       double r = 1.0);

void spherical_spiral_distribution(const unsigned long N,
                                   algebra::SphericalVector3Ds &vs,
                                   double r = 1.0);

//! Distributes N points into a sphere evenly. Fast algorithm that almost
//! guarantees similar distance from one point to the nearest one
void spherical_even_distribution(const unsigned long N,
                                 algebra::SphericalVector3Ds &vs);

void semispherical_even_distribution(const unsigned long N,
                                     algebra::SphericalVector3Ds &vs);

//! Converts a SphericalVector3D proposing a direction of projection
//! in the rotation needed to put such direction in the Z axis.
/**
  \param v is the spherical vector with radius,zenith and azimut
**/
algebra::Rotation3D get_rotation_from_projection_direction(
    const algebra::SphericalVector3D &v);

//! Extract the euler angles from a rotation
/**
  This code assumes than the 1st and 3rd axes are the same
  a1 is the 1st and 3rd axis and a2 is the 2nd. x=0, y=1, z=2.
**/
algebra::Vector3D get_euler_angles_from_rotation(const algebra::Rotation3D &R,
                                                 int a1, int a2);

int next(int axis);
int previous(int axis);

//! convert a quaternion to a rotation matrix.
//! Required by get_euler_angles_from_rotation()
cv::Mat quaternion_to_matrix(const algebra::VectorD<4> &v);

IMPEM2D_END_INTERNAL_NAMESPACE

#endif /* IMPEM2D_INTERNAL_ROTATION_HELPER_H */
