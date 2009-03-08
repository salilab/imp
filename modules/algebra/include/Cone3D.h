/**
 *  \file  Cone3D.h
 *  \brief stores a cone
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */
#ifndef IMPALGEBRA_CONE_3D_H
#define IMPALGEBRA_CONE_3D_H

#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/Sphere3D.h>
#include <iostream>
#include <IMP/constants.h>


IMPALGEBRA_BEGIN_NAMESPACE
//! Cone3D
class IMPALGEBRAEXPORT Cone3D
{
 public:
  Cone3D(){}
  Cone3D(const Vector3D &tip,const Vector3D &direction,
         double angle,double hight=-1);
  /** */
  Vector3D get_tip() const {return tip_;}
  /** */
  Vector3D get_direction() const {return direction_;}
  /** */
  double get_angle() const {return angle_;}
  bool get_contains(const Vector3D &v) const;
  Sphere3D bounding_sphere() const;
  bool is_bound() const {return (hight_ > 0);}
 private:
  Vector3D tip_;
  Vector3D direction_;
  double hight_;
  double   angle_;
};

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_CONE_3D_H */
