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

IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE

inline const Vector4Ds get_derivative_of_composed(
    Vector4D Q, Vector4D P, bool wrt_first = true) {
  Vector4Ds ret;
  ret.reserve(4);
  Vector3D q, p, s, ds_dq, ds_dq0, ds0_dq, p0I3_m_px;
  double qnorm, q0, p0, s0, sign;

  if (!wrt_first) {
    sign = -1;
    qnorm = P.get_magnitude();
    Q = Q.get_unit_vector();
    q = Vector3D(P[1] / qnorm, P[2] / qnorm, P[3] / qnorm);
    p = Vector3D(Q[1], Q[2], Q[3]);
    q0 = P[0] / qnorm;
    p0 = Q[0];
  } else {
    sign = 1;
    qnorm = Q.get_magnitude();
    P = P.get_unit_vector();
    q = Vector3D(Q[1] / qnorm, Q[2] / qnorm, Q[3] / qnorm);
    p = Vector3D(P[1], P[2], P[3]);
    q0 = Q[0] / qnorm;
    p0 = P[0];
  }

  s0 = q0 * p0 - q * p;
  if (s0 < 0) {
    // account for compose() canonicalizing rotation
    p0 = -p0;
    p *= -1;
    s0 = -s0;
  }
  s = p0 * q + q0 * p + sign * get_vector_product(q, p);

  ds_dq0 = p - q0 * s;
  ret.push_back(Vector4D(p0 - q0 * s0, ds_dq0[0], ds_dq0[1], ds_dq0[2]) / qnorm);

  ds0_dq = -p - s0 * q;
  for (unsigned int i = 1; i < 4; ++i) {
    switch (i) {
      case 1:
        p0I3_m_px = Vector3D(p0, -sign * p[2], sign * p[1]);
        break;
      case 2:
        p0I3_m_px = Vector3D(sign * p[2], p0, -sign * p[0]);
        break;
      case 3:
        p0I3_m_px = Vector3D(-sign * p[1], sign * p[0], p0);
        break;
    }
    ds_dq = p0I3_m_px - q[i - 1] * s;
    ret.push_back(Vector4D(ds0_dq[i - 1], ds_dq[0], ds_dq[1], ds_dq[2]) / qnorm);
  }

  return ret;
}

IMPALGEBRA_END_INTERNAL_NAMESPACE

#endif /* IMPALGEBRA_INTERNAL_QUATERNION_DERIVATIVES_H */
