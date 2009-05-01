/**
 *  \file  SphericalCoords.h
 *  \brief stores and converts sperhical coordinates
 *  \author Javier Velazquez-Muriel
 *  Copyright 2007-9 Sali Lab. All rights reserved.
*/
#ifndef IMPALGEBRA_SPHERICAL_COORDS_H
#define IMPALGEBRA_SPHERICAL_COORDS_H

#include "config.h"
#include "Vector3D.h"
#include "IMP/exception.h"
#include "IMP/constants.h"
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
  SphericalCoords() {};

  //! Constructor that directly converts to spherical coordinates from a vector
  //! v in cartesian coordinates
  SphericalCoords(Vector3D &v) {
    from_cartesian(v);
  }

  //! Direct Constructor. A check a check for the validity of the coords is done
  //! by default
  /**
    \param[in] apply_check set it to false if you do not want the check
  **/
  //! v in cartesian coordinates
  SphericalCoords(double r, double tetha, double psi,bool apply_check=true) {
    if(apply_check) {
      if(!check(r,tetha,psi)) {
        String msg = "SphericalCoords:: wrong SphericalCoords coordinates." ;
        throw ValueException(msg.c_str());
      }
    }
    _v[0] = r;
    _v[1] = tetha;
    _v[2] = psi;
  }

  double operator[](unsigned int i) const {
    IMP_assert(i < 3, "Invalid component of SphericalCoords requested: "
               << i << " of " << 3);
    return _v[i];
  }

  double& operator[](unsigned int i) {
    IMP_assert(i < 3, "Invalid component of SphericalCoords requested: "
               << i << " of " << 3);
    return _v[i];
  }

  //! Checks the validity of the coordinates (true if they are correct)
  bool check(double r, double tetha, double psi);

  //! Retunrs a vector with the cartesian coordinates
  Vector3D to_cartesian();

  //! converts a vector in cartesian coordinates to spherical coordinates
  void from_cartesian(Vector3D& v);

 private:
  double _v[3];
};

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_SPHERICAL_COORDS_H */
