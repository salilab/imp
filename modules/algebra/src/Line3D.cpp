/**
 *  \file  Line3D.cpp
 *  \brief simple implementation of lines in 3D
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */
#include <IMP/algebra/Line3D.h>
IMPALGEBRA_BEGIN_NAMESPACE
Line3D::Line3D(const Vector3D &start,const Vector3D &end) {
  start_=start;
  end_=end;
}
double Line3D::lenght() const {
  return (start_-end_).get_magnitude();
}
IMPALGEBRA_END_NAMESPACE
