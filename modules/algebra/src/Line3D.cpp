/**
 *  \file  Line3D.cpp
 *  \brief Simple implementation of lines in 3D
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */
#include <IMP/algebra/Line3D.h>
#include <cmath>

IMPALGEBRA_BEGIN_NAMESPACE

Line3D::Line3D(const Vector3D &direction, const Vector3D &point_on_line)
  : l_(direction.get_unit_vector()) {
    m_ = get_vector_product(point_on_line, l_);
  }

Line3D::Line3D(const algebra::Segment3D &s) : l_(s.get_direction()) {
    m_ = get_vector_product(s.get_point(0), l_);
}

double Line3D::get_reciprocal_product(const Line3D &l) const {
  return l_.get_scalar_product(l.get_moment()) +
         l.get_direction().get_scalar_product(m_);
}

algebra::Segment3D Line3D::get_segment_starting_at(const Vector3D &v,
                                                   double d) const {
  Vector3D start = get_projected((*this), v);
  return Segment3D(start, start + d * l_);
}

Vector3D get_projected(const Line3D &l, const Vector3D &p) {
  return l.get_point_on_line() +
         l.get_direction().get_scalar_product(p) * l.get_direction();
}

algebra::Segment3D get_projected(const Line3D &l, const algebra::Segment3D &s) {
  return Segment3D(get_projected(l, s.get_point(0)),
                   get_projected(l, s.get_point(1)));
}

double get_distance(const Line3D &s, const Vector3D &p) {
  return s.get_moment(p).get_magnitude();
}

double get_angle(const Line3D &a, const Line3D &b) {
  double cosangle = a.get_direction().get_scalar_product(b.get_direction());
  cosangle = (cosangle > 1.) ? 1. : ((cosangle < -1.) ? -1. : cosangle);
  return std::acos(cosangle);
}

double get_distance(const Line3D &a, const Line3D &b) {
  double cross_norm = get_vector_product(a.get_direction(),
                                         b.get_direction()).get_magnitude();
  if (cross_norm > 1e-6) {
    return a.get_reciprocal_product(b) / cross_norm;
  } else {  // lines are parallel
    return get_vector_product(
      a.get_direction(), (a.get_moment() - b.get_moment())).get_magnitude();
  }
}

algebra::Segment3D get_segment_connecting_first_to_second(const Line3D &a,
                                                          const Line3D &b) {
  Vector3D cross, start;
  double cross_norm;
  cross = get_vector_product(a.get_direction(), b.get_direction());
  cross_norm = cross.get_magnitude();
  if (cross_norm > 1e-6) {
    Vector3D a_to_b = -a.get_reciprocal_product(b) * cross /
                       cross_norm / cross_norm;
    start = (-get_vector_product(a.get_moment(),
                                 get_vector_product(b.get_direction(), cross)) +
             b.get_moment().get_scalar_product(cross) *
             a.get_direction()
            ) / cross_norm / cross_norm;
    return Segment3D(start, start + a_to_b);
  } else { // lines are parallel
    start = a.get_point_on_line();
    return Segment3D(start, get_projected(b, start));
  }
}

IMPALGEBRA_END_NAMESPACE
