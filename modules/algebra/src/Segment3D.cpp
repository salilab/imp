/**
 *  \file  Segment3D.cpp
 *  \brief simple implementation of segments in 3D
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */
#include <IMP/algebra/shortest_segment.h>
IMPALGEBRA_BEGIN_NAMESPACE
Segment3D::Segment3D(const Vector3D &start, const Vector3D &end) {
  p_[0] = start;
  p_[1] = end;
}

double Segment3D::get_length() const { return (p_[0] - p_[1]).get_magnitude(); }
/*
namespace {
Vector3D get_projection(const Segment3D &s, const Vector3D &p) {
  Vector3D d = s.get_direction().get_unit_vector();
  double t = d*(s.get_point(0)-p);
  return Vector3D(p +t * d);
}
}*/

/**  Returns f, the 'relative' projection of a point p onto the line
     that contains s.
     Formally, the projection of p onto the line through s is s[0]+f*(s[1]-s[0])
     f is in the range [0..1] if the projection of p is inside s.

     @param s segment in 3D
     @param p point in 3D

     @return the 'relative' project of p onto the line containing s
*/
double get_relative_projection_on_segment(const Segment3D &s,
                                          const algebra::Vector3D &p) {
  algebra::Vector3D vs = s.get_point(1) - s.get_point(0);
  algebra::Vector3D vps = p - s.get_point(0);
  double f = vps * vs / (vs * vs);
  return f;
}

double get_distance(const Segment3D &s, const Vector3D &p) {
  Segment3D ss = get_shortest_segment(s, p);
  return (ss.get_point(0) - ss.get_point(1)).get_magnitude();
}

double get_distance(const Segment3D &a, const Segment3D &b) {
  Segment3D s = get_shortest_segment(a, b);
  return (s.get_point(0) - s.get_point(1)).get_magnitude();
}

IMPALGEBRA_END_NAMESPACE
