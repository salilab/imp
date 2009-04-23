/**
 *  \file  SphericalCoords.h
 *  \brief stores sperhical coordinates
 *  \author Javier Velazquez-Muriel
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */
#ifndef IMPALGEBRA_SPHERICAL_COORDS_H
#define IMPALGEBRA_SPHERICAL_COORDS_H

#include "Vector3D.h"
#include <cmath>


IMPALGEBRA_BEGIN_NAMESPACE

//! Class to represent a 3D point in spherical coordinates
/**
  The order assumed for the representation is radial distance, zenith,
  and azimuth (r, tetha, psi).
  zenith - angle with axis z
  azimuth - angle with axis x
*/

class IMPALGEBRAEXPORT SphericalCoords: public UninitializedDefault
{
public:
  //! Empty constructor
  SphericalCoords();

  //! Constructor that directly converts to spherical coordinates from a vector
  //! v in cartesian coordinates
  SphericalCoords(Vector3D &v) {
    from_cartesian(v);
  }

  double operator[](unsigned int i) const {
    IMP_assert(i < 3, "Invalid component of SphericalCoords requested: "
               << i << " of " << 3);
    return _v[i];
  }

  //! Retunrs a vector with the cartesian coordinates
  inline Vector3D to_cartesian() {
    return Vector3D(_v[0]*cos(_v[2])*sin(_v[1]),
                    _v[0]*sin(_v[2])*sin(_v[1]),
                    _v[0]*cos(_v[1]));
  }

  //! converts a vector in cartesian coordinates to spherical coordinates
  inline void from_cartesian(Vector3D& v) {
    double r =   v.get_magnitude();
    _v[0] = r;
    _v[1] = atan2(v[1],v[0]);
    _v[2] = atan2(sqrt(v[0]*v[0]+v[1]*v[1]),v[2]);
  }

 private:
  double _v[3];
};

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_SPHERICAL_COORDS_H */
