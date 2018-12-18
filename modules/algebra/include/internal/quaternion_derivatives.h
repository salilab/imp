/**
 *  \file quaternion_derivatives.h
 *  \brief Derivatives of quaternions
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
*/

#ifndef IMPALGEBRA_INTERNAL_QUATERNION_DERIVATIVES_H
#define IMPALGEBRA_INTERNAL_QUATERNION_DERIVATIVES_H

#include <IMP/algebra/algebra_config.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/VectorD.h>
#include <Eigen/Dense>

IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE

//! Get 4x4 matrix that projects a matrix to the tangent space of q.
inline Eigen::Matrix4d get_projection(
    const Eigen::Vector4d &q) {
  return Eigen::Matrix4d::Identity() - q * q.transpose();
}

//! Get the skew-symmetric matrix that is equivalent to the cross product.
/** The result is \f$v_\times\f$ such that \f$v \times a = v_\times a\f$,
    where \f$a\f$ and \f$v\f$ are vectors.
 */
inline Eigen::Matrix3d get_cross_matrix(
    const Eigen::Vector3d &v) {
  Eigen::Matrix3d vcross;
  vcross << 0, -v[2], v[1],
            v[2], 0, -v[0],
            -v[1], v[0], 0;
  return vcross;
}

//! Get gradient of rotated vector wrt quaternion of rotation.
/** \param[in] Q quaternion of rotation (assumed to be normalized)
    \param[in] v vector to be rotated by rotation R(Q)
    \param[in] projected Project gradient onto tangent space to Q.
                         Equivalent to differentiating wrt Q/||Q||
                         instead of Q.
 */
inline Eigen::Matrix<double,3,4> get_gradient_of_rotated(
    const Eigen::Vector4d& Q, const Eigen::Vector3d& v, bool projected = true) {
  Eigen::Matrix<double,3,4> dRv_dq;
  Eigen::Matrix3d vcross;
  Eigen::Vector3d q = Q.tail(3);
  double q0 = Q[0];
  vcross = get_cross_matrix(v);
  dRv_dq.col(0) = 2 * (q0 * v + q.cross(v));
  dRv_dq.rightCols(3) =
    2 * (-v * q.transpose()
         + v.dot(q) * Eigen::Matrix3d::Identity() 
         + q * v.transpose()
         - q0 * vcross);

  return (!projected) ? dRv_dq : dRv_dq * get_projection(Q);
}

//! Get gradient of Hamilton product of two quaternions wrt first quaternion.
/** The combined quaternion is S = Q P.
    \param[in] Q first quaternion of rotation (assumed to be normalized)
    \param[in] P second quaternion of rotation (assumed to be normalized)
    \param[in] projected Project gradient onto tangent space to Q.
                         Equivalent to differentiating wrt Q/||Q||
                         instead of Q.
 */
inline Eigen::Matrix4d
  get_gradient_of_composed_wrt_first(
      const Eigen::Vector4d& Q, const Eigen::Vector4d& P,
      bool projected = true) {
  Eigen::Matrix4d dqp_dq;
  dqp_dq.leftCols(1) = P;
  dqp_dq.topRightCorner(1, 3) = -P.tail(3).transpose();
  dqp_dq.bottomRightCorner(3, 3) =
    P[0] * Eigen::Matrix3d::Identity()
    - get_cross_matrix(P.tail(3));
  return (!projected) ? dqp_dq : dqp_dq * get_projection(Q);
}

//! Get gradient of Hamilton product of two quaternions wrt second quaternion.
/** The combined quaternion is S = Q P.
    \param[in] Q first quaternion of rotation (assumed to be normalized)
    \param[in] P second quaternion of rotation (assumed to be normalized)
    \param[in] projected Project gradient onto tangent space to P.
                         Equivalent to differentiating wrt P/||P||
                         instead of P.
 */
inline Eigen::Matrix4d
  get_gradient_of_composed_wrt_second(
      const Eigen::Vector4d& Q, const Eigen::Vector4d& P,
      bool projected = true) {
  Eigen::Matrix4d dqp_dp;
  dqp_dp.leftCols(1) = Q;
  dqp_dp.topRightCorner(1, 3) = -Q.tail(3).transpose();
  dqp_dp.bottomRightCorner(3, 3) =
    Q[0] * Eigen::Matrix3d::Identity()
    + get_cross_matrix(Q.tail(3));
  return (!projected) ? dqp_dp : dqp_dp * get_projection(P);
}

IMPALGEBRA_END_INTERNAL_NAMESPACE

#endif /* IMPALGEBRA_INTERNAL_QUATERNION_DERIVATIVES_H */
