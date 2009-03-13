/**
 *  \file Sphere3DPatch.cpp   \brief Simple 3D sphere patch class.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */
#include "IMP/algebra/Sphere3DPatch.h"
IMPALGEBRA_BEGIN_NAMESPACE
Sphere3DPatch::Sphere3DPatch(const Sphere3D &sph,
  const Plane3D& crossing_plane) {
  crossing_plane_ = crossing_plane;
  sph_=sph;
}
bool Sphere3DPatch::get_contains(const Vector3D &p) const {
  if (!sph_.get_contains(p)) {
    return false;
  }
  //check that the point is above the plane (the direction on the normal)
  return (crossing_plane_.is_above(p));
}
Vector3D Sphere3DPatch::point_on_sphere() const
  //v - a vector on the plane
  Vector3D v =
     orthogonal_vector(crossing_plane_.get_normal()).get_unit_vector();
  Vector3D p = crossing_plane_.get_point_on_plane(); // a point on the plane
  //a point q is both on the plane and on the sphere if q*q=R
  double c = p*p-sph_.get_radius()*sph_.get_radius();
  double b = 2.*p*v;
  double a = v*v;
  double f = (-b+std::sqrt(b*b-4*a*c))/(2*a);
  IMP_check(!std::isnan(f), "problem calculating a point on a sphere a : "
        << a << " b : "<< b << " c : " << c << " f : " << f,ErrorException);
  return p+f*v;
}
IMPALGEBRA_END_NAMESPACE
