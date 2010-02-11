/**
 *  \file  Cylinder3D.h
 *  \brief stores a cylinder
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */
#ifndef IMPALGEBRA_CYLINDER_3D_H
#define IMPALGEBRA_CYLINDER_3D_H

#include "Vector3D.h"
#include "Rotation3D.h"
#include "Transformation3D.h"
#include "Segment3D.h"
#include <iostream>
#include <IMP/constants.h>


IMPALGEBRA_BEGIN_NAMESPACE
/** It does what is says.

    \geometry
*/
class IMPALGEBRAEXPORT Cylinder3D {
 public:
  Cylinder3D(){}
  Cylinder3D(const Segment3D &s, double radius);
  double get_radius() const {return radius_;}
  const Segment3D& get_segment() const {
    return s_;
  }
  IMP_SHOWABLE_INLINE({out << s_ << ": " << radius_;})

 private:
  Segment3D s_;
  double radius_;
};

IMP_VOLUME_GEOMETRY_METHODS(Cylinder3D,
                            return 2.0*PI*g.get_radius()
                            * g.get_segment().get_length()
                            + 2.0*PI *square(g.get_radius()),
                            return PI *square(g.get_radius())
                            * g.get_segment().get_length(),
                            IMP_NOT_IMPLEMENTED);

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_CYLINDER_3D_H */
