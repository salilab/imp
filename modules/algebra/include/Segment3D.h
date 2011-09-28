/**
 *  \file  Segment3D.h
 *  \brief simple implementation of segments in 3D
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */
#ifndef IMPALGEBRA_SEGMENT_3D_H
#define IMPALGEBRA_SEGMENT_3D_H

#include "Vector3D.h"
#include "BoundingBoxD.h"
#include "algebra_macros.h"
#include <iostream>
#include "constants.h"


IMPALGEBRA_BEGIN_NAMESPACE
/** It does what is says.
    \geometry
 */
class IMPALGEBRAEXPORT Segment3D
{
 public:
  Segment3D(){}
  Segment3D(const Vector3D &start,const Vector3D &end);
  //! Get the start=0/end=1 point of the segment
  const Vector3D& get_point(unsigned int i) const {
#if defined(IMP_SWIG_WRAPPER)
    IMP_USAGE_CHECK(i<2,"invalid point index");
#else
    IMP_INTERNAL_CHECK(i<2, "Invalid point index");
#endif
    return p_[i];
  }
  Vector3D get_middle_point() const {
    return .5*p_[0]+ .5*p_[1];
  }
  /** \brief Get a normalized direction vector pointing from
      get_point(0) to get_point(1).
  */
  Vector3D get_direction() const {return (p_[1]-p_[0]).get_unit_vector();}
  double get_length() const;
  IMP_SHOWABLE_INLINE(Segment3D,
                      {out << spaces_io(p_[0]) << ": " << spaces_io(p_[1]);});
 private:
  Vector3D p_[2];
};

IMP_LINEAR_GEOMETRY_METHODS(Segment3D, segment_3d,
                            return BoundingBoxD<3>(g.get_point(0))
                            +BoundingBoxD<3>(g.get_point(1)));



//! Get the distance between a segment and a point
/** \relatesalso Segment3D */
IMPALGEBRAEXPORT double get_distance(const Segment3D &s, const Vector3D &p);

//! Get the distance between two segments
/** \relatesalso Segment3D */
IMPALGEBRAEXPORT double get_distance(const Segment3D &a, const Segment3D &b);


IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_SEGMENT_3D_H */
