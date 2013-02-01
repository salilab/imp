/**
 *  \file SpherePatch3D.cpp   \brief Simple 3D sphere patch class.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/algebra/SpherePatch3D.h"

IMPALGEBRA_BEGIN_NAMESPACE
SpherePatch3D::SpherePatch3D(const Sphere3D &sph,
                             const Plane3D& crossing_plane):
  sph_(sph), crossing_plane_(crossing_plane)
 {
}
bool SpherePatch3D::get_contains(const Vector3D &p) const {
  //check that the point is above the plane (the direction on the normal)
  return (crossing_plane_.get_is_above(p));
}
Vector3D SpherePatch3D::get_boundary_point() const {
  //v - a vector on the plane
  Vector3D
    v = get_orthogonal_vector(crossing_plane_.get_normal()).get_unit_vector();
  Vector3D p = crossing_plane_.get_point_on_plane(); // a point on the plane
  Vector3D cen = sph_.get_center();
  //a point q is both on the plane and on the sphere if (q-center)*(q-center)=R
  double c = -2.*cen*p+cen*cen+p*p-sph_.get_radius()*sph_.get_radius();
  double b = 2.*(p*v+cen*v);
  double a = v*v;
  double f = (-b+std::sqrt(b*b-4*a*c))/(2*a);
  IMP_USAGE_CHECK(!base::isnan(f),
                  "problem calculating a point on a sphere a : "
        << a << " b : "<< b << " c : " << c << " f : " << f);
  return p+f*v;
}
IMPALGEBRA_END_NAMESPACE
