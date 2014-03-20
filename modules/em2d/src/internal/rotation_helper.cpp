/**
 *  \file rotation_helper.cp
 *  \brief Funtions related with rotations in em2d
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/internal/rotation_helper.h"
#include <IMP/constants.h>

IMPEM2D_BEGIN_INTERNAL_NAMESPACE

void semispherical_spiral_distribution(const unsigned long N,
                                       algebra::SphericalVector3Ds &vs,
                                       double r) {
  vs.resize(N);
  double zenith, azimuth;
  for (unsigned long k = 1; k <= N; ++k) {
    double h = 2 * (static_cast<double>(k) - 1) / (2 * N - 1) - 1.0;
    zenith = acos(h);
    if (k == 1) {
      azimuth = 0;
    } else {
      azimuth = (vs[k - 2][2] +
                 3.6 / sqrt(static_cast<double>(N) * 2 * (1.0 - h * h)));
      int div = azimuth / (2 * PI);
      azimuth -= div * 2 * PI;
    }
    // Set the values the spherical vector
    vs[k - 1][0] = r;
    vs[k - 1][1] = zenith;
    vs[k - 1][2] = azimuth;
  }
}

void spherical_spiral_distribution(const unsigned long N,
                                   algebra::SphericalVector3Ds &vs,
                                   double radius) {
  vs.resize(N);
  double zenith, azimuth;
  for (unsigned long k = 1; k <= N; ++k) {
    double h = -1 + 2 * (static_cast<double>(k) - 1) / (N - 1);
    zenith = acos(h);
    if (k == 1 || k == N) {
      azimuth = 0;
    } else {
      azimuth =
          (vs[k - 2][2] + 3.6 / sqrt(static_cast<double>(N) * (1 - h * h)));
      //       int div = azimuth/(2*PI);
      //       azimuth -= div*2*PI;
      int div = static_cast<int>(azimuth / (2 * PI));
      azimuth -= static_cast<double>(div) * (2 * PI);
    }
    // Set the values the spherical vector
    vs[k - 1][0] = radius;
    vs[k - 1][1] = zenith;
    //    vs[k-1][1]=(-1)*zenith;
    vs[k - 1][2] = azimuth;
  }
}

// It is based on a different algorithm than the previous function
void spherical_even_distribution(const unsigned long N,
                                 algebra::SphericalVector3Ds &vs) {
  vs.resize(N);
  double ratio = (1 + sqrt(5.)) / 2;  // golden ratio
  double increment = 2 * PI / ratio;  // how much to increment the longitude
  double dz = 2. / static_cast<double>(N);  // unit sphere has diameter 2
  for (unsigned i = 0; i < N; ++i) {
    double z = i * dz - 1 + dz / 2.;  //  z location of each band/point
    double radius = sqrt(1 - z * z);
    double az = i * increment;  // azimuth
    double x = radius * cos(az);
    double y = radius * sin(az);
    algebra::Vector3D v(x, y, z);
    vs[i] = algebra::SphericalVector3D(v);
  }
}

void semispherical_even_distribution(const unsigned long N,
                                     algebra::SphericalVector3Ds &vs) {
  vs.resize(0);
  algebra::SphericalVector3Ds temp(2 * N);
  spherical_even_distribution(2 * N, temp);
  for (unsigned long i = 0; i < 2 * N; ++i) {
    if (temp[i][1] < (PI / 2)) {
      vs.push_back(temp[i]);
    }
  }
  // Check
  if (vs.size() < N) {
    std::cerr << "semispherical_even_distribution: less than N points "
                 "created." << std::endl;
  }
}

algebra::Rotation3D get_rotation_from_projection_direction(
    const algebra::SphericalVector3D &v) {
  // The rotation is obtained after inverting the zenith angle vs[1] in the
  // direction of the y-axis that would result after rotating an azimuth v[2]
  algebra::Vector3D axis(-sin(v[2]), cos(v[2]), 0);
  // return get_rotation_in_radians_about_axis(axis,-v[1]);
  return algebra::get_rotation_about_axis(axis, -v[1]);
}

cv::Mat quaternion_to_matrix(const algebra::VectorD<4> &v) {
  cv::Mat M(3, 3, CV_64FC1);
  M.at<double>(0, 0) = v[0] * v[0] + v[1] * v[1] - v[2] * v[2] - v[3] * v[3];
  M.at<double>(0, 1) = 2 * (v[1] * v[2] - v[0] * v[3]);
  M.at<double>(0, 2) = 2 * (v[1] * v[3] + v[0] * v[2]);
  M.at<double>(1, 0) = 2 * (v[1] * v[2] + v[0] * v[3]);
  M.at<double>(1, 1) = v[0] * v[0] - v[1] * v[1] + v[2] * v[2] - v[3] * v[3];
  M.at<double>(1, 2) = 2 * (v[2] * v[3] - v[0] * v[1]);
  M.at<double>(2, 0) = 2 * (v[1] * v[3] - v[0] * v[2]);
  M.at<double>(2, 1) = 2 * (v[2] * v[3] + v[0] * v[1]);
  M.at<double>(2, 2) = v[0] * v[0] - v[1] * v[1] - v[2] * v[2] + v[3] * v[3];
  return M;
}

// Herter, T. Computers & Graphics Volume 17, Issue 5, September-October 1993
algebra::Vector3D get_euler_angles_from_rotation(const algebra::Rotation3D &R,
                                                 int a1, int a2) {
  algebra::VectorD<4> q = R.get_quaternion();
  cv::Mat M = quaternion_to_matrix(q);
  double epsilon = 1e-4;
  double theta, phi, psi, cos_auxiliar_angle, sin_auxiliar_angle;
  bool positive_orientation = true;
  int c = 0;
  if (a2 == next(a1)) {
    positive_orientation = true;
    a2 = next(a1);
    c = next(a2);
  } else if (a2 == previous(a1)) {
    positive_orientation = false;
    a2 = previous(a1);
    c = previous(a2);
  }
  // Adjust indices for C++ matrix access
  a1 -= 1;
  a2 -= 1;
  c -= 1;

  theta = acos(M.at<double>(a1, a1));
  if (abs(M.at<double>(a1, a1)) < (1 - epsilon)) {
    cos_auxiliar_angle = M.at<double>(a1, c);
    if (!positive_orientation) {
      cos_auxiliar_angle = -cos_auxiliar_angle;
    }
    phi = atan2(M.at<double>(a1, a2), cos_auxiliar_angle);
    sin_auxiliar_angle = M.at<double>(a2, a1);
    cos_auxiliar_angle = M.at<double>(c, a1);
    if (positive_orientation) {
      cos_auxiliar_angle = -cos_auxiliar_angle;
    }
  } else {
    phi = 0.0;
    cos_auxiliar_angle = M.at<double>(a2, a2);
    sin_auxiliar_angle = M.at<double>(c, a2);
    if (!positive_orientation) {
      sin_auxiliar_angle = -sin_auxiliar_angle;
    }
  }
  psi = atan2(sin_auxiliar_angle, cos_auxiliar_angle);
  algebra::Vector3D euler_angles(phi, theta, psi);
  return euler_angles;
}

int next(int axis) {
  axis += 1;
  if (axis > 3) {
    return 1;
  } else {
    return axis;
  }
}

int previous(int axis) {
  axis -= 1;
  if (axis == 0) {
    return 3;
  } else {
    return axis;
  }
}

IMPEM2D_END_INTERNAL_NAMESPACE
