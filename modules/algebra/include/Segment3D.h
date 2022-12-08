/**
 *  \file IMP/algebra/Segment3D.h
 *  \brief Simple implementation of segments in 3D
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#ifndef IMPALGEBRA_SEGMENT_3D_H
#define IMPALGEBRA_SEGMENT_3D_H

#include <IMP/algebra/algebra_config.h>
#include "Vector3D.h"
#include "BoundingBoxD.h"
#include "algebra_macros.h"
#include "GeometricPrimitiveD.h"
#include <iostream>
#include <boost/serialization/access.hpp>
#include "constants.h"

IMPALGEBRA_BEGIN_NAMESPACE
//! Simple implementation of segments in 3D
/** A line segment is defined by a start and end point (as Vector3D).
    \geometry
 */
class IMPALGEBRAEXPORT Segment3D : public GeometricPrimitiveD<3> {
 public:
  Segment3D() {}
  Segment3D(const Vector3D &start, const Vector3D &end);
  //! Get the start (i==0) or end (i==1) point of the segment
  const Vector3D &get_point(unsigned int i) const {
    IMP_INTERNAL_CHECK(i < 2, "Invalid point index");
    return p_[i];
  }
  Vector3D get_middle_point() const { return .5 * p_[0] + .5 * p_[1]; }
  //! Get a normalized direction vector from get_point(0) to get_point(1).
  Vector3D get_direction() const { return (p_[1] - p_[0]).get_unit_vector(); }
  double get_length() const;
  IMP_SHOWABLE_INLINE(Segment3D,
  { out << spaces_io(p_[0]) << ": " << spaces_io(p_[1]); });

 private:
  Vector3D p_[2];

  friend class boost::serialization::access;

  template<class Archive> void serialize(Archive &ar, const unsigned int) {
    ar & p_[0] & p_[1];
  }
};

IMP_LINEAR_GEOMETRY_METHODS(Segment3D, segment_3d,
                            return BoundingBoxD<3>(g.get_point(0)) +
                                   BoundingBoxD<3>(g.get_point(1)));

//! Return the 'relative' projection of a point p onto the line that contains s.
/** Formally, the projection of p onto the line through s is s[0]+f*(s[1]-s[0])
    f is in the range [0..1] if the projection of p is inside s.

    @param s segment in 3D
    @param p point in 3D

    @return the 'relative' projection of p onto the line containing s
*/
IMPALGEBRAEXPORT double get_relative_projection_on_segment(
    const Segment3D &s, const algebra::Vector3D &p);

//! Get the distance between a segment and a point
/** \see Segment3D */
IMPALGEBRAEXPORT double get_distance(const Segment3D &s, const Vector3D &p);

//! Get the distance between two segments
/** \see Segment3D */
IMPALGEBRAEXPORT double get_distance(const Segment3D &a, const Segment3D &b);

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_SEGMENT_3D_H */
