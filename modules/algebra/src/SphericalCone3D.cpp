/**
 *  \file  SphericalCone3D.cpp
 *  \brief simple implementation of cones in 3D
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */
#include <IMP/algebra/SphericalCone3D.h>
IMPALGEBRA_BEGIN_NAMESPACE

IMPALGEBRAEXPORT SphericalCone3D cone_from_line(const Line3D &l,double angle) {
  SphericalCone3D cone(l.get_start_point(),l.get_direction(),
                       angle,l.lenght());
  return cone;
}
SphericalCone3D::SphericalCone3D(const Vector3D &tip,const Vector3D &direction,
                                 double angle, double hight) {
  tip_=tip;
  direction_=direction;
  IMP_assert(angle >= 0,"input angle is out of range");
  IMP_assert(angle <= PI,"input angle is out of range");
  angle_=angle;
  hight_=hight;
}
bool SphericalCone3D::get_contains(const Vector3D &v) const {
  if (is_bound()) {
    if (distance(v,tip_)>hight_) {
      return false;
    }
  }
  Vector3D d = (v - tip_).get_unit_vector();
  double  x = d*direction_;
  return (x > 0) && (x >= std::cos(angle_));
}
Sphere3D SphericalCone3D::get_bounding_sphere() const {
  IMP_assert(is_bound(),"can not bound an open cone");
  return Sphere3D(tip_,hight_);
}
IMPALGEBRA_END_NAMESPACE
