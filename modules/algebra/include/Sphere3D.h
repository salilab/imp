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
private:
  Vector3D center_;
  double radius_;
};

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_SPHERE_3D_H */
