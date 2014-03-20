/**
 *  \file dihedral_helpers.h
 *  \brief Helper functions to calculate dihedral angles
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_DIHEDRAL_HELPERS_H
#define IMPCORE_DIHEDRAL_HELPERS_H

#include <IMP/core/core_config.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/core/XYZ.h>
#include <cmath>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

//! Calculate the dihedral angle between the given four XYZ particles.
/** \return the dihedral angle.
    If derv0 is non-nullptr, all four algebra::Vector3D parameters are filled
    in on return with the derivatives with respect to the XYZ particles.
 */
template <class P>
inline double dihedral(const P &d0, const P &d1, const P &d2, const P &d3,
                       algebra::Vector3D *derv0, algebra::Vector3D *derv1,
                       algebra::Vector3D *derv2, algebra::Vector3D *derv3) {
  algebra::Vector3D rij = get_vector_geometry(d0) - get_vector_geometry(d1);
  algebra::Vector3D rkj = get_vector_geometry(d2) - get_vector_geometry(d1);
  algebra::Vector3D rkl = get_vector_geometry(d2) - get_vector_geometry(d3);

  algebra::Vector3D v1 = get_vector_product(rij, rkj);
  algebra::Vector3D v2 = get_vector_product(rkj, rkl);
  double scalar_product = v1.get_scalar_product(v2);
  double mag_product = v1.get_magnitude() * v2.get_magnitude();

  // avoid division by zero
  double cosangle =
      std::abs(mag_product) > 1e-12 ? scalar_product / mag_product : 0.0;

  // avoid range error for acos
  cosangle = std::max(std::min(cosangle, static_cast<Float>(1.0)),
                      static_cast<Float>(-1.0));

  double angle = std::acos(cosangle);
  // get sign
  algebra::Vector3D v0 = get_vector_product(v1, v2);
  double sign = rkj * v0;
  if (sign < 0.0) {
    angle = -angle;
  }

  if (derv0) {
    // method for derivative calculation from van Schaik et al.
    // J. Mol. Biol. 234, 751-762 (1993)
    algebra::Vector3D vijkj = get_vector_product(rij, rkj);
    algebra::Vector3D vkjkl = get_vector_product(rkj, rkl);
    double sijkj2 = vijkj.get_squared_magnitude();
    double skjkl2 = vkjkl.get_squared_magnitude();
    double skj = rkj.get_magnitude();
    double rijkj = rij * rkj;
    double rkjkl = rkj * rkl;

    double fact0 = sijkj2 > 1e-8 ? skj / sijkj2 : 0.0;
    double fact1 = skj > 1e-8 ? rijkj / (skj * skj) : 0.0;
    double fact2 = skj > 1e-8 ? rkjkl / (skj * skj) : 0.0;
    double fact3 = skjkl2 > 1e-8 ? -skj / skjkl2 : 0.0;

    *derv0 = fact0 * vijkj;
    *derv3 = fact3 * vkjkl;
    *derv1 = (fact1 - 1.0) * (*derv0) - fact2 * (*derv3);
    *derv2 = (fact2 - 1.0) * (*derv3) - fact1 * (*derv0);
  }
  return angle;
}

IMPCORE_END_INTERNAL_NAMESPACE

#endif /* IMPCORE_DIHEDRAL_HELPERS_H */
