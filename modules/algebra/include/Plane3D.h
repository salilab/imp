/**
 *  \file Plane3D.h   \brief Simple 3D plane class.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
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
class Plane3D {
public:
  Plane3D(){}
  Plane3D(const VectorD<3>& point_on_plane,const VectorD<3> &normal_to_plane) {
    normal_ = normal_to_plane;
    distance_= normal_*point_on_plane;
    IMP_USAGE_CHECK(std::abs(normal_.get_squared_magnitude()-1) < .05,
                    "The normal vector must be normalized");
  }
  Plane3D(double distance_to_plane ,const VectorD<3> &normal_to_plane):
    distance_(distance_to_plane),
    normal_(normal_to_plane){
    IMP_USAGE_CHECK(std::abs(normal_.get_squared_magnitude()-1) < .05,
                    "The normal vector must be normalized");
  }
  VectorD<3> get_point_on_plane() const {return normal_*distance_;}
  const VectorD<3> &get_normal() const {return normal_;}
  //! Project the point onto the plane
  VectorD<3> get_projected(const VectorD<3> &p) const  {
    return p-normal_*(normal_*p-distance_);
  }
#ifndef IMP_DOXYGEN
  VectorD<3> get_projection(const VectorD<3> &p) const  {
    return get_projected(p);
  }
#endif
  /** @name Orientation
       Up is the direction of the normal. You really shouldn't use
       these as they aren't very reliable.
       @{
  */
  bool get_is_above(const VectorD<3> &p) const {
    return get_height(p) > 0;
  }
  bool get_is_below(const VectorD<3> &p) const {
    return get_height(p) < 0;
  }
  /** @} */
  double get_height(const VectorD<3> &p) const {
    return normal_*p-distance_;
  }
  IMP_SHOWABLE_INLINE(Plane3D, {
      out << "(" << distance_ << ": " << spaces_io(normal_)
          << ")";
    });

  //! Return the plane with the opposite normal
  Plane3D get_opposite() const {
    return Plane3D(-distance_, -normal_);
  }
  double get_distance_from_origin() const {
    return distance_;
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


//! return the point reflected about the plane
inline VectorD<3> get_reflected(const Plane3D &pln, const VectorD<3> &p) {
  VectorD<3> proj= pln.get_projected(p);
  return p+2*(proj-p);
}


IMP_AREA_GEOMETRY_METHODS(Plane3D, plane_3d,
                          if (0) std::cout << g;
                          return std::numeric_limits<double>::infinity(),
                          {
                            if (0) std::cout << g;
                            VectorD<3> ip
                              = get_ones_vector_d<3>(
                                   std::numeric_limits<double>::infinity());
                            return BoundingBoxD<3>(ip)+ BoundingBox3D(-ip);
                          });
IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_PLANE_3D_H */
