/**
 *  \file IMP/algebra/Line3D.h
 *  \brief Simple implementation of lines in 3D
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 */

#ifndef IMPALGEBRA_LINE_3D_H
#define IMPALGEBRA_LINE_3D_H

#include <IMP/algebra/algebra_config.h>
#include "Vector3D.h"
#include "Segment3D.h"
#include "BoundingBoxD.h"
#include "algebra_macros.h"
#include "GeometricPrimitiveD.h"

IMPALGEBRA_BEGIN_NAMESPACE
//! Simple implementation of lines in 3D
/** A line is defined by a direction and any point on the
    line. Internally, Plucker coordinates are used, where
    the line is defined by a direction and an orthogonal
    moment about the origin whose magnitude is the
    distance from the origin.
    \geometry
    \see Segment3D
 */
class IMPALGEBRAEXPORT Line3D : public GeometricPrimitiveD<3> {
  Vector3D l_, m_;

  public:
    Line3D() {}
    Line3D(const Vector3D &direction, const Vector3D &point_on_line);
    //! Create line along segment.
    Line3D(const algebra::Segment3D &s);

    //! Get the unit vector in the direction of the line.
    const Vector3D& get_direction() const { return l_; }

    //! Get the point on the line closest to the origin.
    Vector3D get_point_on_line() const { return get_vector_product(l_, m_); }

    //! Get the moment of the line about the origin.
    Vector3D get_moment() const { return m_; }

    //! Get the moment of the line about a point.
    Vector3D get_moment(const Vector3D &v) const {
      return m_ - get_vector_product(v, l_);
    }

    //! Get reciprocal (or virtual) product, the moment of either line about the other.
    double get_reciprocal_product(const Line3D &l) const;

    //! Get the line in the opposite direction.
    Line3D get_opposite() const { return Line3D(-l_, get_point_on_line()); }

    //! Get segment of a given length starting at a point.
    /** \note The point is not assumed to be on the line and is therefore
        projected onto the line to construct the segment.
    */
    algebra::Segment3D get_segment_starting_at(const Vector3D &v, double d) const;

    IMP_SHOWABLE_INLINE(Line3D,
    { out << "[" << l_ << " ; " << m_ << "]"; });
};

IMP_LINEAR_GEOMETRY_METHODS(Line3D, line_3d,
                            Vector3D pop = g.get_point_on_line();
                            return BoundingBoxD<3>(pop) +
                                   BoundingBoxD<3>(pop + g.get_direction()));

//! Project a point onto the line.
/** This is equivalent to the point on the line closest to the
    provided point.
*/
IMPALGEBRAEXPORT Vector3D get_projected(const Line3D &l, const Vector3D &p);

//! Project a segment onto a line.
IMPALGEBRAEXPORT algebra::Segment3D get_projected(const Line3D &l,
                                                  const algebra::Segment3D &s);

//! Get closest distance between a line and a point.
IMPALGEBRAEXPORT double get_distance(const Line3D &s, const Vector3D &p);

//! Get angle in radians between two lines around their closest points.
IMPALGEBRAEXPORT double get_angle(const Line3D &a, const Line3D &b);

//! Get the closest distance between two lines.
IMPALGEBRAEXPORT double get_distance(const Line3D &a, const Line3D &b);

//! Get shortest possible segment from the first line to the second.
/** \note If the lines are parallel, this segment is not unique and is chosen
//  so that the segment passes closest to the origin.*/
IMPALGEBRAEXPORT algebra::Segment3D get_segment_connecting_first_to_second(
  const Line3D &a, const Line3D &b);

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_LINE_3D_H */
