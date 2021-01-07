/**
 *  \file Rotation3D.cpp   \brief Simple 3D rotation class.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/algebra/Rotation3D.h"
#include <IMP/algebra/internal/quaternion_derivatives.h>
#include "IMP/algebra/VectorD.h"
#include "IMP/algebra/vector_generators.h"
#include "IMP/algebra/utility.h"
#include <IMP/check_macros.h>
#include <IMP/log_macros.h>
#include <limits>


IMPALGEBRA_BEGIN_NAMESPACE

Rotation3D get_rotation_from_matrix(Eigen::Matrix3d m) {
  return get_rotation_from_matrix(m(0, 0), m(0, 1), m(0, 2), m(1, 0), m(1, 1),
                                  m(1, 2), m(2, 0), m(2, 1), m(2, 2));
}

Rotation3D get_rotation_from_matrix(double m11, double m12, double m13,
                                    double m21, double m22, double m23,
                                    double m31, double m32, double m33) {
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    Vector3D v0(m11, m12, m13);
    Vector3D v1(m21, m22, m23);
    Vector3D v2(m31, m32, m33);
    IMP_USAGE_CHECK(std::abs(v0.get_squared_magnitude() - 1) < .1,
                    "The passed matrix is not a rotation matrix (row 0).");
    IMP_USAGE_CHECK(std::abs(v1.get_squared_magnitude() - 1) < .1,
                    "The passed matrix is not a rotation matrix (row 1).");
    IMP_USAGE_CHECK(std::abs(v2.get_squared_magnitude() - 1) < .1,
                    "The passed matrix is not a rotation matrix (row 2).");
    IMP_USAGE_CHECK(
        std::abs(v0 * v1) < .1,
        "The passed matrix is not a rotation matrix (row 0, row 1).");
    IMP_USAGE_CHECK(
        std::abs(v0 * v2) < .1,
        "The passed matrix is not a rotation matrix (row 0, row 2).");
    IMP_USAGE_CHECK(
        std::abs(v1 * v2) < .1,
        "The passed matrix is not a rotation matrix (row 1, row 2).");
    Vector3D c0(m11, m21, m31);
    Vector3D c1(m12, m22, m32);
    Vector3D c2(m13, m23, m33);
    IMP_USAGE_CHECK(std::abs(c0.get_squared_magnitude() - 1) < .1,
                    "The passed matrix is not a rotation matrix (col 0).");
    IMP_USAGE_CHECK(std::abs(c1.get_squared_magnitude() - 1) < .1,
                    "The passed matrix is not a rotation matrix (col 1).");
    IMP_USAGE_CHECK(std::abs(c2.get_squared_magnitude() - 1) < .1,
                    "The passed matrix is not a rotation matrix (col 2).");
    IMP_USAGE_CHECK(
        std::abs(c0 * c1) < .1,
        "The passed matrix is not a rotation matrix (col 0, col 1).");
    IMP_USAGE_CHECK(
        std::abs(c0 * c2) < .1,
        "The passed matrix is not a rotation matrix (col 0, col 2).");
    IMP_USAGE_CHECK(
        std::abs(c1 * c2) < .1,
        "The passed matrix is not a rotation matrix (col 1, col 2).");
#if IMP_HAS_CHECKS >= IMP_USAGE
    double det = m11 * (m22 * m33 - m23 * m32) + m12 * (m23 * m31 - m21 * m33) +
                 m13 * (m21 * m32 - m22 * m31);
#endif
    IMP_USAGE_CHECK(std::abs(det - 1) < .1,
                    "The determinant of the rotation "
                    "matrix is not 1. Got "
                        << det);
  }
  Rotation3D ret;
  {
    double a, b, c, d;
    a = std::abs(1 + m11 + m22 + m33) / 4;
    b = std::abs(1 + m11 - m22 - m33) / 4;
    c = std::abs(1 - m11 + m22 - m33) / 4;
    d = std::abs(1 - m11 - m22 + m33) / 4;

    // make sure quat is normalized.
    double sum = a + b + c + d;
    a = std::sqrt(a / sum);
    b = std::sqrt(b / sum);
    c = std::sqrt(c / sum);
    d = std::sqrt(d / sum);

    if (m32 - m23 < 0.0) b = -b;
    if (m13 - m31 < 0.0) c = -c;
    if (m21 - m12 < 0.0) d = -d;
    ret = Rotation3D(a, b, c, d);
#if 0
    IMP_IF_LOG(VERBOSE) {
      Vector3D xr = ret.get_rotated(get_basis_vector_d<3>(0));
      Vector3D yr = ret.get_rotated(get_basis_vector_d<3>(1));
      Vector3D zr = ret.get_rotated(get_basis_vector_d<3>(2));
      IMP_LOG_TERSE("Got:\n");
      IMP_LOG_TERSE(xr[0] << " " << yr[0] << " " << zr[0] << std::endl);
      IMP_LOG_TERSE(xr[1] << " " << yr[1] << " " << zr[1] << std::endl);
      IMP_LOG_TERSE(xr[2] << " " << yr[2] << " " << zr[2] << std::endl);
    }
#endif
  }
  return ret;
}

void Rotation3D::get_rotated_adjoint(const Vector3D &x, const Vector3D &Dy,
                                     Vector3D *Dx, Rotation3DAdjoint *DQ) const {
  // Convert w=R(r)v to y=R(Q)x
  double q0 = v_[0];
  const Vector3D q(v_[1], v_[2], v_[3]);
  double qDy = q * Dy;

  if (Dx) {
    Vector3D qcrossDy = get_vector_product(q, Dy);
    double cos_theta = 2 * q0 * q0 - 1;
    *Dx = cos_theta * Dy + 2 * (qDy * q - q0 * qcrossDy); // R(q)^T Dy
  }

  if (DQ) {
    double xDy = x * Dy;
    Vector3D xcrossDy = get_vector_product(x, Dy);
    Vector3D Dq = 2 * (-xDy * q + q0 * xcrossDy + q * x * Dy + qDy * x); // J_q(R(q0,q)x)^T Dy
    (*DQ)[0] = 2 * (q0 * xDy + q * xcrossDy);
    std::copy(Dq.begin(), Dq.end(), DQ->begin() + 1);
  }
}

RotatedVector3DAdjoint
Rotation3D::get_rotated_adjoint(const Vector3D &v, const Vector3D &Dw) const {
  Vector3D Dv;
  Rotation3DAdjoint Dr;
  get_rotated_adjoint(v, Dw, &Dv, &Dr);
  return RotatedVector3DAdjoint(Dv, Dr);
}

Vector3D Rotation3D::get_gradient_of_rotated(const Vector3D &v,
                                             unsigned int i,
                                             bool wrt_unnorm) const {
  IMP_USAGE_CHECK(i < 4, "Invalid derivative component.");
  Eigen::Vector4d q(v_.get_data());
  Eigen::Vector3d V(v.get_data());
  Eigen::Matrix<double,3,4> dRv_dq = internal::get_jacobian_of_rotated(
    q, V, wrt_unnorm);
  Vector3D dRv_dqi;
  Eigen::VectorXd::Map(&dRv_dqi[0], 3) = dRv_dq.col(i);
  return dRv_dqi;
}

Vector3D Rotation3D::get_derivative(const Vector3D &v,
                                    unsigned int i,
                                    bool wrt_unnorm) const {
  IMPALGEBRA_DEPRECATED_METHOD_DEF(
    2.12,
    "Use get_gradient_of_rotated(args) instead."
  );
  return get_gradient_of_rotated(v, i, wrt_unnorm);
}

Eigen::MatrixXd Rotation3D::get_jacobian_of_rotated(
    const Eigen::Vector3d &v, bool wrt_unnorm) const {
  Eigen::Vector4d q(v_.get_data());
  return internal::get_jacobian_of_rotated(q, v, wrt_unnorm);
}

Eigen::MatrixXd Rotation3D::get_gradient(
  const Eigen::Vector3d &v, bool wrt_unnorm) const {
  IMPALGEBRA_DEPRECATED_METHOD_DEF(
    2.12,
    "Use get_jacobian_of_rotated(args) instead."
  );
  return get_jacobian_of_rotated(v, wrt_unnorm);
}

void compose_adjoint(const Rotation3D &RA, const Rotation3D &RB, Vector4D DC,
                     Rotation3DAdjoint *DA, Rotation3DAdjoint *DB) {
  const Vector4D A = RA.get_quaternion();
  const Vector4D B = RB.get_quaternion();

  // account for compose() canonicalizing rotation
  if ((A[0] * B[0] - A[1] * B[1] - A[2] * B[2] - A[3] * B[3]) < 0)
    DC *= -1;

  Eigen::Map<const Eigen::Vector3d> Dc(DC.get_data() + 1);

  if (DA) {
    Eigen::Map<const Eigen::Vector3d> b(B.begin() + 1);
    Eigen::Map<Eigen::Vector3d> Da(DA->begin() + 1);
    (*DA)[0] = B[0] * DC[0] + b.dot(Dc);
    Da = -DC[0] * b + B[0] * Dc + b.cross(Dc);
  }

  if (DB) {
    Eigen::Map<const Eigen::Vector3d> a(A.begin() + 1);
    Eigen::Map<Eigen::Vector3d> Db(DB->begin() + 1);
    (*DB)[0] = A[0] * DC[0] + a.dot(Dc);
    Db = -DC[0] * a + A[0] * Dc - a.cross(Dc);
  }
}

ComposeRotation3DAdjoint
compose_adjoint(const Rotation3D &A, const Rotation3D &B, const Rotation3DAdjoint &DC) {
  Rotation3DAdjoint DA, DB;
  compose_adjoint(A, B, DC, &DA, &DB);
  return ComposeRotation3DAdjoint(DA, DB);
}

Eigen::MatrixXd get_jacobian_of_composed_wrt_first(
    const Rotation3D &q, const Rotation3D &p, bool wrt_unnorm) {
  Eigen::Vector4d Q(q.get_quaternion().get_data());
  Eigen::Vector4d P(p.get_quaternion().get_data());
  if (Q[0] * P[0] - Q.tail(3).dot(P.tail(3)) < 0) {
    // account for compose() canonicalizing rotation
    P *= -1;
  }
  return internal::get_jacobian_of_composed_wrt_first(Q, P, wrt_unnorm);
}

Eigen::MatrixXd get_gradient_of_composed_with_respect_to_first(
  const Rotation3D &q, const Rotation3D &p, bool wrt_unnorm) {
  IMPALGEBRA_DEPRECATED_FUNCTION_DEF(2.12, "Use get_jacobian_of_composed_wrt_first(args) instead.");
  return get_jacobian_of_composed_wrt_first(q, p, wrt_unnorm);
}

Eigen::MatrixXd get_jacobian_of_composed_wrt_second(
    const Rotation3D &q, const Rotation3D &p, bool wrt_unnorm) {
  Eigen::Vector4d Q(q.get_quaternion().get_data());
  Eigen::Vector4d P(p.get_quaternion().get_data());
  if (Q[0] * P[0] - Q.tail(3).dot(P.tail(3)) < 0) {
    // account for compose() canonicalizing rotation
    Q *= -1;
  }
  return internal::get_jacobian_of_composed_wrt_second(Q, P, wrt_unnorm);
}

Eigen::MatrixXd get_gradient_of_composed_with_respect_to_second(
  const Rotation3D &q, const Rotation3D &p, bool wrt_unnorm) {
  IMPALGEBRA_DEPRECATED_FUNCTION_DEF(2.12, "Use get_jacobian_of_composed_with_respect_to_second(args) instead.");
  return get_jacobian_of_composed_wrt_second(q, p, wrt_unnorm);
}

Rotation3D get_random_rotation_3d() {
  VectorD<4> r = get_random_vector_on<4>(get_unit_sphere_d<4>());
  return Rotation3D(r[0], r[1], r[2], r[3]);
}

Rotation3Ds get_uniform_cover_rotations_3d(unsigned int n) {
  // "surface area" is 2 pi^2 r^3= 2pi^2.
  // each rotation has an area of approximately 4/3 pi distance^3
  Vector<VectorD<4> > vs =
      internal::uniform_cover_sphere<4>(n, get_zero_vector_d<4>(), 1, false);
  Rotation3Ds ret;
  for (unsigned int i = 0; i < vs.size(); ++i) {
    if (vs[i][0] <= 0) {
      vs[i] = -vs[i];
    }
    ret.push_back(Rotation3D(vs[i][0], vs[i][1], vs[i][2], vs[i][3]));
  }
  return ret;
}

Rotation3D get_random_rotation_3d(const Rotation3D &center, double distance) {
  unsigned int count = 0;
  double d2 = get_squared(distance);
  while (count < 10000) {
    Rotation3D rr = get_random_rotation_3d();
    if (get_distance(center, rr) < d2) {
      return rr;
    }
    ++count;
  }
  IMP_FAILURE("Unable to find a suitably close rotation");
}

Rotation3D get_rotation_from_fixed_xyz(double xr, double yr, double zr) {
  double a, b, c, d;
  double cx = cos(xr);
  double cy = cos(yr);
  double cz = cos(zr);
  double sx = sin(xr);
  double sy = sin(yr);
  double sz = sin(zr);
  double m00 = cz * cy;
  double m11 = -sy * sx * sz + cx * cz;
  double m22 = cy * cx;
  double zero = 0.0;
  a = std::sqrt(std::max(1 + m00 + m11 + m22, zero)) / 2.0;
  b = std::sqrt(std::max(1 + m00 - m11 - m22, zero)) / 2.0;
  c = std::sqrt(std::max(1 - m00 + m11 - m22, zero)) / 2.0;
  d = std::sqrt(std::max(1 - m00 - m11 + m22, zero)) / 2.0;
  if (cy * sx + sy * cx * sz + sx * cz < 0.0) b = -b;
  if (sz * sx - sy * cx * cz - sy < 0.0) c = -c;
  if (sz * cy + sy * sx * cz + sz * cx < 0.0) d = -d;
  return Rotation3D(a, b, c, d);
}

Rotation3D get_rotation_from_fixed_zxz(double phi, double theta, double psi) {
  double a, b, c, d;
  double c1, c2, c3, s1, s2, s3;
  c2 = std::cos(theta / 2);
  c1 = cos(phi / 2);
  c3 = cos(psi / 2);
  s2 = std::sin(theta / 2);
  s1 = sin(phi / 2);
  s3 = sin(psi / 2);
  a = c1 * c2 * c3 + s1 * s2 * s3;
  b = s1 * c2 * c3 - c1 * s2 * s3;
  c = c1 * s2 * c3 + s1 * c2 * s3;
  d = c1 * c2 * s3 - s1 * s2 * c3;
  return Rotation3D(a, b, c, d);
}

Rotation3D get_rotation_from_fixed_zyz(double Rot, double Tilt, double Psi) {
  double c1 = std::cos(Rot);
  double c2 = std::cos(Tilt);
  double c3 = std::cos(Psi);
  double s1 = std::sin(Rot);
  double s2 = std::sin(Tilt);
  double s3 = std::sin(Psi);

  /*IMP_LOG_VERBOSE( "Intermediates front: "
          << c1 << " " << c2 << " " << c3 << "\n"
          << s1 << " " << s2 << " " << s3 << std::endl);*/
  double d00 = c1 * c2 * c3 - s1 * s3;
  double d01 = (-1.0) * c2 * c3 * s1 - c1 * s3;
  double d02 = c3 * s2;
  double d10 = c3 * s1 + c1 * c2 * s3;
  double d11 = c1 * c3 - c2 * s1 * s3;
  double d12 = s2 * s3;
  double d20 = (-1.0) * c1 * s2;
  double d21 = s1 * s2;
  double d22 = c2;
  Rotation3D rot =
      get_rotation_from_matrix(d00, d01, d02, d10, d11, d12, d20, d21, d22);
  return rot;
}

FixedXYZ get_fixed_xyz_from_rotation(const Rotation3D &r) {
  VectorD<4> quat = r.get_quaternion();
  double q00 = get_squared(quat[0]);
  double q11 = get_squared(quat[1]);
  double q22 = get_squared(quat[2]);
  double q33 = get_squared(quat[3]);
  double mat11 = q00 + q11 - q22 - q33;
  double mat21 = 2 * (quat[1] * quat[2] + quat[0] * quat[3]);
  // double mat23 = 2*(quat[2]*quat[3] - quat[0]*quat[1]);
  double mat31 = 2 * (quat[1] * quat[3] - quat[0] * quat[2]);
  double mat32 = 2 * (quat[2] * quat[3] + quat[0] * quat[1]);
  double mat33 = q00 - q11 - q22 + q33;
  return FixedXYZ(
      std::atan2(mat32, mat33),
      std::atan2(mat31, std::sqrt(std::pow(mat21, 2) + std::pow(mat11, 2))),
      std::atan2(mat21, mat11));
}

Rotation3D get_rotation_taking_first_to_second(const Vector3D &v1,
                                               const Vector3D &v2) {
  Vector3D v1_norm = v1.get_unit_vector();
  Vector3D v2_norm = v2.get_unit_vector();
  // get a vector that is perpendicular to the plane containing v1 and v2
  Vector3D vv = get_vector_product(v1_norm, v2_norm);
  // get the angle between v1 and v2
  double dot = v1_norm * v2_norm;
  dot = (dot < -1.0 ? -1.0 : (dot > 1.0 ? 1.0 : dot));
  double angle = std::acos(dot);
  // check a special case: the input vectors are parallel / antiparallel
  if (std::abs(dot) > .999999) {
    IMP_LOG_VERBOSE(" the input vectors are (anti)parallel " << std::endl);
    return get_rotation_about_axis(get_orthogonal_vector(v1), angle);
  }
  return get_rotation_about_axis(vv, angle);
}

Rotation3D get_rotation_from_x_y_axes(const Vector3D &x, const Vector3D &y) {
  IMP_USAGE_CHECK(std::abs(x.get_squared_magnitude() - 1.0) < .1,
                  "The x vector is not a unit vector.");
  IMP_USAGE_CHECK(std::abs(y.get_squared_magnitude() - 1.0) < .1,
                  "The y vector is not a unit vector.");
  IMP_USAGE_CHECK(std::abs(x * y) < .1,
                  "The x and y vectors are not perpendicular.");
  Vector3D z = get_vector_product(x, y);
  Rotation3D rot = get_rotation_from_matrix(x[0], y[0], z[0], x[1], y[1], z[1],
                                            x[2], y[2], z[2]);
  return rot;
}

std::pair<Vector3D, double> get_axis_and_angle(const Rotation3D &rot) {
  VectorD<4> q = rot.get_quaternion();
  double a, b, c, d;
  a = q[0];
  b = q[1];
  c = q[2];
  d = q[3];
  //  deal with singularity of a~1.0 first
  double numerical_scale_factor(1.0+std::abs(a));
  if ( std::abs(a) >= 1.0 - 4.0 * std::numeric_limits<double>::epsilon() * numerical_scale_factor ){
    return std::make_pair(Vector3D(1, 0, 0), 0.0);
  }
  //if (std::abs(a) > .9999)
  //  return std::make_pair(Vector3D(1, 0, 0), 0.0);
  double angle = std::acos(a) * 2;
  double s = std::sin(angle / 2);
  Vector3D axis(b / s, c / s, d / s);
  return std::make_pair(axis.get_unit_vector(), angle);
  // VectorD<4> q = rot.get_quaternion();
  // const double &cos_half = q[0];
  // const double sin_half = std::sqrt(q[1]*q[1] + q[2]*q[2] + q[3]*q[3]);
  // if(sin_half>0.0){ // numerically stable solution:
  //   double angle = 2 * std::atan2(sin_half, cos_half);
  //   Vector3D axis(q[1] / sin_half,
  //                 q[2] / sin_half,
  //                 q[3]/ sin_half);
  //   return std::make_pair(axis.get_unit_vector(), angle);
  // } else {
  //    return std::make_pair(Vector3D(1, 0, 0), 0.0);
  // }
}

//! Generates a nondegenerate set of Euler angles with a delta resolution
algebra::Rotation3Ds get_uniformly_sampled_rotations(double delta_rad) {
  double delta = delta_rad / PI * 180.0;
  Vector3D eu_start(0., 0., 0.);  // psi,theta,phi
  Vector3D eu_end(360., 180., 360.);
  Vector3D eu_range = eu_end - eu_start;
  double phi_steps = get_rounded((eu_range[2] / delta) + 0.499);
  double phi_real_dist = eu_range[2] / phi_steps;
  double theta_steps = get_rounded((eu_range[1] / delta) + 0.499);
  double theta_real_dist = eu_range[1] / theta_steps;
  double angle2rad = PI / 180.;
  double psi_steps, psi_ang_dist, psi_real_dist;

  algebra::Rotation3Ds ret;
  for (double phi = eu_start[2]; phi < eu_end[2]; phi += phi_real_dist) {
    for (double theta = eu_start[1]; theta <= eu_end[1];
         theta += theta_real_dist) {
      if (theta == 0.0 || theta == 180.0) {
        psi_steps = 1;
      } else {
        psi_steps =
            get_rounded(360.0 * std::cos((90.0 - theta) * angle2rad) / delta);
      }
      psi_ang_dist = 360.0 / psi_steps;
      psi_real_dist = eu_range[0] / (ceil(eu_range[0] / psi_ang_dist));
      for (double psi = eu_start[0]; psi < eu_end[0]; psi += psi_real_dist) {
        ret.push_back(get_rotation_from_fixed_zxz(
            phi * angle2rad, theta * angle2rad, psi * angle2rad));
      }
    }
  }
  return ret;
}

IMPALGEBRA_END_NAMESPACE
