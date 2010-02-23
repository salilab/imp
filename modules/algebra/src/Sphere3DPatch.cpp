/**
 *  \file SpherePatch3D.cpp   \brief Simple 3D sphere patch class.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include <IMP/utility.h>
#include "IMP/algebra/SpherePatch3D.h"

IMPALGEBRA_BEGIN_NAMESPACE
SpherePatch3D::SpherePatch3D(const SphereD<3> &sph,
  const Plane3D& crossing_plane) {
  crossing_plane_ = crossing_plane;
  sph_=sph;
}
bool SpherePatch3D::get_contains(const VectorD<3> &p) const {
  //check that the point is above the plane (the direction on the normal)
  return (crossing_plane_.get_is_above(p));
}
VectorD<3> SpherePatch3D::get_boundary_point() const {
  //v - a vector on the plane
  VectorD<3>
    v = orthogonal_vector(crossing_plane_.get_normal()).get_unit_vector();
  VectorD<3> p = crossing_plane_.get_point_on_plane(); // a point on the plane
  VectorD<3> cen = sph_.get_center();
  //a point q is both on the plane and on the sphere if (q-center)*(q-center)=R
  double c = -2.*cen*p+cen*cen+p*p-sph_.get_radius()*sph_.get_radius();
  double b = 2.*(p*v+cen*v);
  double a = v*v;
  double f = (-b+std::sqrt(b*b-4*a*c))/(2*a);
  IMP_USAGE_CHECK(!is_nan(f), "problem calculating a point on a sphere a : "
        << a << " b : "<< b << " c : " << c << " f : " << f);
  return p+f*v;
}
IMPALGEBRA_END_NAMESPACE
