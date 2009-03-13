/**
 *  \file  Segment3D.cpp
 *  \brief simple implementation of segments in 3D
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */
#include <IMP/algebra/Segment3D.h>
IMPALGEBRA_BEGIN_NAMESPACE
Segment3D::Segment3D(const Vector3D &start,const Vector3D &end) {
  start_=start;
  end_=end;
}
Vector3D Segment3D::get_point(unsigned int i) const {
  IMP_check(i<2,"invalid point index",ValueException);
  if (i==0) return start_;
  return end_;
}
double Segment3D::lenght() const {
  return (start_-end_).get_magnitude();
}
Vector3D Segment3D::projection(const Vector3D &p) const{
  Vector3D d = get_direction().get_unit_vector();
  double t = d*(start_-p);
  return Vector3D(start_+t * d);
}

double Segment3D::distance(const Vector3D &p) const{
  return (start_-projection(p)).get_magnitude();
}

IMPALGEBRA_END_NAMESPACE
