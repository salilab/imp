/**
 *  \file  Cone3D.h
 *  \brief stores a cone
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#ifndef IMPALGEBRA_CONE_3D_H
#define IMPALGEBRA_CONE_3D_H

#include "Vector3D.h"
#include "Sphere3D.h"
#include "Sphere3DPatch.h"
#include "Plane3D.h"
#include "Segment3D.h"
#include <iostream>
#include <IMP/constants.h>


IMPALGEBRA_BEGIN_NAMESPACE
/**
   In general, a cone is a pyramid with a circular cross section.
   A right cone is a cone with its vertex above the center of its base.
   However, when used without qualification, the term "cone" often means
   "right cone." We have implemented a "right cone".
   \geometry
 */
class IMPALGEBRAEXPORT Cone3D
{
 public:
  // A cone with a top at s.get_point(0) and the given base radius
  Cone3D(const Segment3D &s,double radius);
  //!Get the vertex of the cone
  Vector3D get_tip() const {return seg_.get_point(0);}
  //!Get the direction of the axis of the cone
  /** This vector points from the tip into the occupied volume.
   */
  Vector3D get_direction() const {return seg_.get_direction();}
  double get_height() const {return seg_.get_length();}
  /** The opening angle of a cone is the vertex angle made by a cross section
      through the apex and center of the base.*/
  double get_angle() const {
    return  2.*std::atan(radius_ / get_height());
  }
  //! get the radius of the base circle
  double get_radius() const {return radius_;}
  bool get_contains(const Vector3D &v) const;
  //! Get the plane supporting the base of the cone
  /** The cone is on the positive side of the plane.
   */
  Plane3D get_base_plane() const;
  void show(std::ostream &out) const {
    out << seg_ << ": " << radius_;
  }
 private:
  Segment3D seg_;
  double radius_;
};

IMP_VOLUME_GEOMETRY_METHODS(Cone3D, IMP_NOT_IMPLEMENTED,
                            IMP_NOT_IMPLEMENTED,
                            IMP_NOT_IMPLEMENTED);
IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_CONE_3D_H */
