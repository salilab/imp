/**
 *  \file anglel_helpers.h
 *  \brief Helper functions to calculate and handle angles
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_ANGLE_HELPERS_H
#define IMPCORE_ANGLE_HELPERS_H

#include <IMP/core/core_config.h>
#include "../XYZ.h"
#include <IMP/constants.h>
#include <IMP/algebra/Vector3D.h>
#include <cmath>

#include <cmath>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

//! Calculate the angle between the given three XYZ particles.
/** \return the angle.
    If derv0 is non-nullptr, all three algebra::Vector3D parameters are filled
    in on return with the derivatives with respect to the XYZ particles.
 */
template <class P>
inline double angle(const P &d0, const P &d1, const P &d2,
                    algebra::Vector3D *derv0, algebra::Vector3D *derv1,
                    algebra::Vector3D *derv2) {
  algebra::Vector3D rij = get_vector_geometry(d0) - get_vector_geometry(d1);
  algebra::Vector3D rkj = get_vector_geometry(d2) - get_vector_geometry(d1);

  double scalar_product = rij * rkj;
  double mag_rij = rij.get_magnitude();
  double mag_rkj = rkj.get_magnitude();
  double mag_product = mag_rij * mag_rkj;

  // avoid division by zero
  double cosangle =
      std::abs(mag_product) > 1e-12 ? scalar_product / mag_product : 0.0;

  // avoid range error for acos
  cosangle = std::max(std::min(cosangle, static_cast<double>(1.0)),
                      static_cast<double>(-1.0));

  double angle = std::acos(cosangle);

  if (derv0) {
    algebra::Vector3D unit_rij = rij.get_unit_vector();
    algebra::Vector3D unit_rkj = rkj.get_unit_vector();

    double sinangle = std::abs(std::sin(angle));

    double fact_ij = sinangle * mag_rij;
    double fact_kj = sinangle * mag_rkj;
    // avoid division by zero
    fact_ij = std::max(static_cast<double>(1e-12), fact_ij);
    fact_kj = std::max(static_cast<double>(1e-12), fact_kj);

    *derv0 = (unit_rij * cosangle - unit_rkj) / fact_ij;
    *derv2 = (unit_rkj * cosangle - unit_rij) / fact_kj;
    *derv1 = -(*derv0) - (*derv2);
  }
  return angle;
}

//! Return the difference between two angles
/** Both angles, and the difference, are given in radians.
    The difference is the shortest distance from angle1 to angle2, and is
    thus in the range -PI to PI.
 */
inline double get_angle_difference(double angle1, double angle2) {
  angle1 = std::fmod(angle1, 2.0 * PI);
  angle2 = std::fmod(angle2, 2.0 * PI);
  double diff = angle2 - angle1;
  return diff < -PI ? diff + 2.0 * PI : diff > PI ? diff - 2.0 * PI : diff;
}

IMPCORE_END_INTERNAL_NAMESPACE

#endif /* IMPCORE_ANGLE_HELPERS_H */
