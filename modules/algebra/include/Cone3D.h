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
#include <IMP/algebra/Sphere3DPatch.h>
#include <IMP/algebra/Plane3D.h>
#include <IMP/algebra/Segment3D.h>
#include <iostream>
#include <IMP/constants.h>


IMPALGEBRA_BEGIN_NAMESPACE
//! Cone3D
/**
In general, a cone is a pyramid with a circular cross section.
A right cone is a cone with its vertex above the center of its base.
However, when used without qualification, the term "cone" often means
"right cone." We have implemented a "right cone"
 */
class IMPALGEBRAEXPORT Cone3D
{
 public:
  Cone3D(const Segment3D &s,double radius);
  //!Get the vertex of the cone
  Vector3D get_tip() const {return seg_.get_point(0);}
  //!Get the direction of the cone
  Vector3D get_direction() const {return seg_.get_direction();}
  double get_height() const {return seg_.lenght();}
  /** The opening angle of a cone is the vertex angle made by a cross section
      through the apex and center of the base.*/
  double get_angle() const {
    return  2.*std::atan(radius_ / get_height());
  }
  bool get_contains(const Vector3D &v) const;
  Sphere3D get_bounding_sphere() const;
  //! Get the intersecting plane between a cone and its bounding sphere
  Plane3D get_intersecting_plane() const;
 private:
  Segment3D seg_;
  double radius_;
};
IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_CONE_3D_H */
