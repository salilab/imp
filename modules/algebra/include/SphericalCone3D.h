/**
 *  \file  SphericalCone3D.h
 *  \brief stores a cone
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPALGEBRA_SPHERICAL_CONE_3D_H
#define IMPALGEBRA_SPHERICAL_CONE_3D_H

#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/Sphere3D.h>
#include <IMP/algebra/Line3D.h>
#include <iostream>
#include <IMP/constants.h>


IMPALGEBRA_BEGIN_NAMESPACE
//! SphericalCone3D
class IMPALGEBRAEXPORT SphericalCone3D
{
 public:
  SphericalCone3D(){}
  SphericalCone3D(const Vector3D &tip,const Vector3D &direction,
         double angle,double hight=-1);
  /** */
  Vector3D get_tip() const {return tip_;}
  /** */
  Vector3D get_direction() const {return direction_;}
  /** */
  double get_angle() const {return angle_;}
  bool get_contains(const Vector3D &v) const;
  Sphere3D get_bounding_sphere() const;
  bool is_bound() const {return (hight_ > 0);}
 private:
  Vector3D tip_;
  Vector3D direction_;
  double hight_;
  double   angle_;
};
IMPALGEBRAEXPORT SphericalCone3D cone_from_line(const Line3D &l,double angle);
IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_SPHERICAL_CONE_3D_H */
