/**
 *  \file IMP/algebra/SphericalVector3D.h
 *  \brief Stores and converts spherical coordinates
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 */

#ifndef IMPALGEBRA_SPHERICAL_VECTOR_3D_H
#define IMPALGEBRA_SPHERICAL_VECTOR_3D_H

#include <IMP/algebra/algebra_config.h>
#include "Vector3D.h"
#include "IMP/exception.h"
#include "constants.h"
#include <IMP/showable_macros.h>
#include "GeometricPrimitiveD.h"
#include <cmath>

IMPALGEBRA_BEGIN_NAMESPACE

//! Class to represent a 3D point in spherical coordinates
/**
  The order assumed for the representation is radial distance, zenith,
  and azimuth (r, theta, psi).
  zenith - angle with axis z
  azimuth - angle with axis x
  \geometry
*/
class IMPALGEBRAEXPORT SphericalVector3D : public GeometricPrimitiveD<3> {
  bool check(double r, double theta, double psi) const;

 public:
  SphericalVector3D() {};

  //! Construct from a Cartesian coordinate vector
  SphericalVector3D(const Vector3D& v) { set_cartesian_coordinates(v); }

  //! Construct from provided spherical coordinates.
  /** A check for the validity of the coords is done by default.
      Set apply_check to false if you do not want the check.
   */
  SphericalVector3D(double r, double theta, double psi,
                    bool apply_check = true) {
    if (apply_check) {
      if (!check(r, theta, psi)) {
        String msg = "SphericalCoords:: wrong SphericalCoords coordinates.";
        throw ValueException(msg.c_str());
      }
    }
    v_[0] = r;
    v_[1] = theta;
    v_[2] = psi;
  }

  double operator[](unsigned int i) const {
    IMP_USAGE_CHECK(i < 3, "Invalid component of SphericalCoords requested: "
                               << i << " of " << 3);
    return v_[i];
  }

  double& operator[](unsigned int i) {
    IMP_USAGE_CHECK(i < 3, "Invalid component of SphericalCoords requested: "
                               << i << " of " << 3);
    return v_[i];
  }

  //! Returns a vector with the Cartesian coordinates
  Vector3D get_cartesian_coordinates() const;

  IMP_SHOWABLE_INLINE(SphericalVector3D, out << v_[0] << " , " << v_[1] << " , "
                                             << v_[2]);

 private:
  void set_cartesian_coordinates(const Vector3D& v);
  double v_[3];
};

IMP_VALUES(SphericalVector3D, SphericalVector3Ds);

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_SPHERICAL_VECTOR_3D_H */
