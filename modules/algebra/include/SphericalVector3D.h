/**
 *  \file  SphericalVector3D.h
 *  \brief Stores and converts spherical coordinates
 *  \author Javier Velazquez-Muriel
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
*/
#ifndef IMPALGEBRA_SPHERICAL_VECTOR_3D_H
#define IMPALGEBRA_SPHERICAL_VECTOR_3D_H

#include "config.h"
#include "Vector3D.h"
#include "IMP/exception.h"
#include "IMP/constants.h"
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
class IMPALGEBRAEXPORT SphericalVector3D
{
  bool check(double r, double theta, double psi);
 public:
  SphericalVector3D() {};

  //! Constructor that directly converts to spherical coordinates from a vector
  //! v in Cartesian coordinates
  SphericalVector3D(VectorD<3> &v) {
    set_cartesian_coordinates(v);
  }

  //! Direct Constructor. A check for the validity of the coords is done
  //! by default
  /**
    Set apply_check to false if you do not want the check
  **/
  SphericalVector3D(double r, double theta,
                    double psi,bool apply_check=true) {
    if(apply_check) {
      if(!check(r,theta,psi)) {
        String msg = "SphericalCoords:: wrong SphericalCoords coordinates." ;
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
  VectorD<3> get_cartesian_coordinates();

 private:
  void set_cartesian_coordinates(VectorD<3>& v);
  double v_[3];
};


inline std::ostream &operator<<(std::ostream &out, const SphericalVector3D &v) {
  out << "( " << v[0] << " , " << v[1] << " , " << v[2] << " )";
  return out;
}


typedef std::vector<SphericalVector3D> SphericalVector3Ds;

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_SPHERICAL_VECTOR_3D_H */
