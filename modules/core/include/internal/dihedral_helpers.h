/**
 *  \file dihedral_helpers.h
 *  \brief Helper functions to calculate dihedral angles
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_DIHEDRAL_HELPERS_H
#define IMPCORE_DIHEDRAL_HELPERS_H

#include "../config.h"
#include "../XYZ.h"
#include <IMP/algebra/Vector3D.h>
#include <cmath>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

//! Calculate the dihedral angle between the given four XYZ particles.
/** \return the dihedral angle.
    If derv0 is non-NULL, all four algebra::VectorD<3> parameters are filled
    in on return with the derivatives with respect to the XYZ particles.
 */
inline double dihedral(const XYZ &d0, const XYZ &d1,
                       const XYZ &d2, const XYZ &d3,
                       algebra::VectorD<3> *derv0, algebra::VectorD<3> *derv1,
                       algebra::VectorD<3> *derv2, algebra::VectorD<3> *derv3)
{
  algebra::VectorD<3> rij = d1.get_vector_to(d0);
  algebra::VectorD<3> rkj = d1.get_vector_to(d2);
  algebra::VectorD<3> rkl = d3.get_vector_to(d2);

  algebra::VectorD<3> v1 = get_vector_product(rij, rkj);
  algebra::VectorD<3> v2 = get_vector_product(rkj, rkl);
  double scalar_product = v1.get_scalar_product(v2);
  double mag_product = v1.get_magnitude() * v2.get_magnitude();

  // avoid division by zero
  double cosangle = std::abs(mag_product) > 1e-12
                    ? scalar_product / mag_product : 0.0;

  // avoid range error for acos
  cosangle = std::max(std::min(cosangle, static_cast<Float>(1.0)),
                      static_cast<Float>(-1.0));

  double angle = std::acos(cosangle);
  // get sign
  algebra::VectorD<3> v0 = get_vector_product(v1, v2);
  double sign = rkj*v0;
  if (sign < 0.0) {
    angle = -angle;
  }

  if (derv0) {
    // method for derivative calculation from van Schaik et al.
    // J. Mol. Biol. 234, 751-762 (1993)
    algebra::VectorD<3> vijkj = get_vector_product(rij, rkj);
    algebra::VectorD<3> vkjkl = get_vector_product(rkj, rkl);
    double sijkj2 = vijkj.get_squared_magnitude();
    double skjkl2 = vkjkl.get_squared_magnitude();
    double skj = rkj.get_magnitude();
    double rijkj = rij*rkj;
    double rkjkl = rkj*rkl;

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

#endif  /* IMPCORE_DIHEDRAL_HELPERS_H */
