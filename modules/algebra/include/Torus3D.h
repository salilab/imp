/**
 *  \file IMP/algebra/Torus3D.h
 *  \brief Represent a torus in 3D.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 */

#ifndef IMPALGEBRA_TORUS_3D_H
#define IMPALGEBRA_TORUS_3D_H

#include "Vector3D.h"
#include "Rotation3D.h"
#include "Transformation3D.h"
#include "Segment3D.h"
#include "GeometricPrimitiveD.h"
#include <IMP/showable_macros.h>
#include <iostream>
#include "constants.h"

IMPALGEBRA_BEGIN_NAMESPACE

//! Represent a torus in 3D.
/**
   \geometry
*/
class IMPALGEBRAEXPORT Torus3D : public GeometricPrimitiveD<3> {
 public:
  Torus3D() {}
  Torus3D (double major_radius, double minor_radius);
  double get_major_radius() const { return R_; }
  double get_minor_radius() const { return r_; }

  /**
     Computes distance and derivative of a sphere s relative
     to the surface of a torus with major radius R and minor radius r (on z=0, the inner and outer
     torus rings have radii R-r and R+r, respectively).

     @return a tuple (d,v) s.t.:
     d = The shortest distance from any point on s to any point on the torus.
     If s penetrates the torus, the opposite of the maximal penetration depth is returnd.
     v = the normalized derivative vector of d
  */
  inline std::pair<double, algebra::Vector3D>
    get_torus_distance(Sphere3D) const;


  IMP_SHOWABLE_INLINE(Torus3D, { out << R_ << " on " << r_; });

 private:
  double R_; // major radius
  double r_; // minor radius
};

IMP_VOLUME_GEOMETRY_METHODS(Torus3D, torus_3d,
                            return 4.0 * get_squared(PI)
                            * g.get_major_radius() * g.get_minor_radius(),
                            return 2.0 * get_squared(PI)
                            * g.get_major_radius()
                            * get_squared(g.get_minor_radius()),
                            IMP_UNUSED(g);
                            IMP_NOT_IMPLEMENTED);


// implememt inline method
std::pair<double, algebra::Vector3D>
  Torus3D::get_torus_distance
  (Sphere3D sphere) const
{
  double const x=sphere[0];
  double const y=sphere[1];
  double const z=sphere[2];
  const double eps = 1e-9;
  double d_xy2 = x*x + y*y;
  double d_xy = std::sqrt(d_xy2);
  double d_tx, d_ty;
  if ( d_xy > eps )
  {
    // (d_tx,d_ty) is the vector from the nearest point on the center line of the torus to (x,y) (projected on z=0)
    d_tx = x - x/d_xy*R_;
    d_ty = y - y/d_xy*R_;
  }
  else
  {
    d_tx = x - R_;
    d_ty = y;
  }

  //   (dx*dxx + dy*dyx) =
  //   = (x-R*cosT)*[1-R*(x*cosT-dxy)/dxy2] + (y-R*sinT)(-R*sinT*x)/dxy2
  //   = x - R*cosT + [ -R*x*(x*cosT-dxy) + R^2*cosT*(x*cosT-dxy) - x*y*R*sinT + x*R^2*sinT^2 ] /dx2
  //   = x - R*cosT + [ -R*x^2*cosT + R*x*dxy + x*R^2*cosT^2 - R^2*cosT*dxy - x*y*R*sinT + x*R^2*sinT^2 ] / dx2
  //   = x - R*cosT + [ -R*x^2*cosT + R*x*dxy + x*R^2 - R^2*x - x*y*R*sinT  ] /dx2
  //   = x - R*cosT + [ -R*x^2*cosT + R*x*x/cosT - x*x*R*sinT^2/cosT  ] /dx2
  //   = x - R*cosT + [ -R*x^2*cosT + R*x*x*cosT  ] /dx2
  //   = x - R*cosT

  if ( d_xy <= R_ )
  {
    double denom = std::sqrt(z*z + d_tx*d_tx + d_ty*d_ty); // magnitude of vector from nearest point on the torus center line to (x,y,z)
    double idenom = 1.0/denom;
    algebra::Vector3D der;
    if ( denom > eps )
    {
      der[0] = d_tx*idenom;
      der[1] = d_ty*idenom;
      der[2] = z*idenom;
    }
    else
    {
      der[0] = d_tx/eps;
      der[1] = d_ty/eps;
      der[2] = z/eps;
    }
    return std::make_pair(denom - r_ - sphere.get_radius(), der);
  }
  else
  {
    double dz = z > 0 ? z - r_ - sphere.get_radius(): z + r_ + sphere.get_radius();
    double denom = std::sqrt(dz*dz + d_tx*d_tx + d_ty*d_ty);
    algebra::Vector3D der;
    if ( denom > eps )
    {
      der[0] = -d_tx/denom;
      der[1] = -d_ty/denom;
      der[2] = dz/denom;
    }
    else
    {
      der[0] = -d_tx/eps;
      der[1] = -d_ty/eps;
      der[2] = dz/eps;
    }
    return std::make_pair(-denom - r_ - sphere.get_radius(), der);
  }
}



IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_TORUS_3D_H */
