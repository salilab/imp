/**
 *  \file  Segment3D.cpp
 *  \brief simple implementation of segments in 3D
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */
#include <IMP/algebra/shortest_segment.h>
#include <IMP/log.h>
IMPALGEBRA_BEGIN_NAMESPACE
Segment3D::Segment3D(const VectorD<3> &start,const VectorD<3> &end) {
  p_[0]=start;
  p_[1]=end;
}

double Segment3D::get_length() const {
  return (p_[0]-p_[1]).get_magnitude();
}

VectorD<3> get_projection(const Segment3D &s, const VectorD<3> &p) {
  VectorD<3> d = s.get_direction().get_unit_vector();
  double t = d*(s.get_point(0)-p);
  return VectorD<3>(p +t * d);
}




double get_distance(const Segment3D &s, const VectorD<3> &p) {
  Segment3D ss= get_shortest_segment(s, p);
  return (ss.get_point(0) - ss.get_point(1)).get_magnitude();
}


double get_distance(const Segment3D &a, const Segment3D &b) {
  Segment3D s= get_shortest_segment(a, b);
  return (s.get_point(0) - s.get_point(1)).get_magnitude();
}

IMPALGEBRA_END_NAMESPACE
