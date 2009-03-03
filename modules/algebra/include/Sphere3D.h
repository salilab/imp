/**
 *  \file Sphere3D.h   \brief Simple 3D sphere class.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */
#ifndef IMPALGEBRA_SPHERE_3D_H
#define IMPALGEBRA_SPHERE_3D_H

#include <IMP/algebra/Cylinder3D.h>

IMPALGEBRA_BEGIN_NAMESPACE

//! Represent a sphere
class IMPALGEBRAEXPORT Sphere3D {
public:
  /** */
  Sphere3D(const Vector3D& center,double radius);
  /** */
  Float get_volume() const;
  /** */
  Float get_surface_area() const;
  /** */
  Float get_radius() const {return radius_;}
  /**  */
  const Vector3D &get_center() const {return center_;}
  /**  */
  Cylinder3D get_bounding_cylinder() const {
    return Cylinder3D(get_center()-Vector3D(0.0,0.0,get_radius()),
                      get_center()+Vector3D(0.0,0.0,get_radius()),
                      get_radius());
  }
  //! Return true if this sphere contains the other one
  bool get_contains(const Sphere3D &o) const {
    double d= (get_center()-o.get_center()).get_magnitude();
    return (d+ o.get_radius() < get_radius());
  }
private:
  Vector3D center_;
  double radius_;
};

//! Return the distance between the two spheres if they are disjoint
/** If they intersect, the distances are not meaningful. */
inline double distance(const Sphere3D& a, const Sphere3D &b) {
  double d= (a.get_center()-b.get_center()).get_magnitude();
  return d - a.get_radius() - b.get_radius();
}

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_SPHERE_3D_H */
