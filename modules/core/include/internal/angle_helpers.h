/**
 *  \file anglel_helpers.h
 *  \brief Helper functions to calculate and handle angles
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_ANGLE_HELPERS_H
#define IMPCORE_ANGLE_HELPERS_H

#include "../config.h"
#include "../XYZ.h"
#include <IMP/constants.h>
#include <IMP/algebra/Vector3D.h>
#include <cmath>

#include <cmath>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

//! Calculate the angle between the given three XYZ particles.
/** \return the angle.
    If derv0 is non-NULL, all three algebra::VectorD<3> parameters are filled
    in on return with the derivatives with respect to the XYZ particles.
 */
inline double angle(const XYZ &d0, const XYZ &d1, const XYZ &d2,
                    algebra::VectorD<3> *derv0, algebra::VectorD<3> *derv1,
                    algebra::VectorD<3> *derv2)
{
  algebra::VectorD<3> rij = d1.get_vector_to(d0);
  algebra::VectorD<3> rkj = d1.get_vector_to(d2);

  double scalar_product = rij*rkj;
  double mag_rij = rij.get_magnitude();
  double mag_rkj = rkj.get_magnitude();
  double mag_product = mag_rij * mag_rkj;

  // avoid division by zero
  double cosangle = std::abs(mag_product) > 1e-12 ? scalar_product / mag_product
                                                  : 0.0;

  // avoid range error for acos
  cosangle = std::max(std::min(cosangle, static_cast<double>(1.0)),
                      static_cast<double>(-1.0));

  double angle = std::acos(cosangle);

  if (derv0) {
    algebra::VectorD<3> unit_rij = rij.get_unit_vector();
    algebra::VectorD<3> unit_rkj = rkj.get_unit_vector();

    double sinangle = std::abs(std::sin(angle));

    double fact_ij = sinangle * mag_rij;
    double fact_kj = sinangle * mag_rkj;
    // avoid division by zero
    fact_ij = std::max(static_cast<double>(1e-12), fact_ij);
    fact_kj = std::max(static_cast<double>(1e-12), fact_kj);

    *derv0 = (unit_rij * cosangle - unit_rkj) / fact_ij;
    *derv2 = (unit_rkj * cosangle - unit_rij) / fact_kj;
    *derv1 = - (*derv0) - (*derv2);
  }
  return angle;
}

//! Return the difference between two angles
/** Both angles, and the difference, are given in radians.
    The difference is the shortest distance from angle1 to angle2, and is
    thus in the range -PI to PI.
 */
double get_angle_difference(double angle1, double angle2)
{
  angle1 = std::fmod(angle1, 2.0 * PI);
  angle2 = std::fmod(angle2, 2.0 * PI);
  double diff = angle2 - angle1;
  return diff < -PI ? diff + 2.0 * PI : diff > PI ? diff - 2.0 * PI : diff;
}

IMPCORE_END_INTERNAL_NAMESPACE

#endif  /* IMPCORE_ANGLE_HELPERS_H */
