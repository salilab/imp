/**
 *  \file Plane3D.h   \brief Simple 3D plane class.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */
#ifndef IMPALGEBRA_PLANE_3D_H
#define IMPALGEBRA_PLANE_3D_H

#include "Vector3D.h"
#include "BoundingBoxD.h"

IMPALGEBRA_BEGIN_NAMESPACE

/** Represent a plane in 3D.
   \geometry
*/
class IMPALGEBRAEXPORT Plane3D {
public:
  Plane3D(){}
  Plane3D(const VectorD<3>& point_on_plane,const VectorD<3> &normal_to_plane);
  Plane3D(double distance_to_plane ,const VectorD<3> &normal_to_plane);
  VectorD<3> get_point_on_plane() const {return normal_*distance_;}
  const VectorD<3> &get_normal() const {return normal_;}
  //! Project the point onto the plane
  VectorD<3> get_projection(const VectorD<3> &p) const;
  /** @name Orientation
       Up is the direction of the normal.
       @{
  */
  /** \cgalpredicate
   */
  bool get_is_above(const VectorD<3> &p) const;
  /** \cgalpredicate
   */
  bool get_is_below(const VectorD<3> &p) const;
  /** @} */
  IMP_SHOWABLE

  //! Return the plane with the opposite normal
  Plane3D get_opposite() const {
    return Plane3D(-distance_, -normal_);
  }
private:
  double distance_;
  VectorD<3> normal_; //normal to plane
};


//! Return the distance between a plane and a point in 3D
/** \relatesalso Plane3D */
inline double get_distance(const Plane3D& pln, const VectorD<3> &p) {
  return (pln.get_projection(p)-p).get_magnitude();
}

IMP_AREA_GEOMETRY_METHODS(Plane3D,
                          return std::numeric_limits<double>::infinity(),
                          {
                            VectorD<3> ip
                              = get_ones_vector_d<3>(
                                   std::numeric_limits<double>::infinity());
                            return BoundingBoxD<3>(ip)+ BoundingBox3D(-ip);
                          });
IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_PLANE_3D_H */
