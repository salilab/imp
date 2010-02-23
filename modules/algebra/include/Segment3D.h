/**
 *  \file  Segment3D.h
 *  \brief simple implementation of segments in 3D
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */
#ifndef IMPALGEBRA_SEGMENT_3D_H
#define IMPALGEBRA_SEGMENT_3D_H

#include "Vector3D.h"
#include "BoundingBoxD.h"
#include "macros.h"
#include <iostream>
#include <IMP/constants.h>


IMPALGEBRA_BEGIN_NAMESPACE
/** It does what is says.
    \geometry
 */
class IMPALGEBRAEXPORT Segment3D
{
 public:
  Segment3D(){}
  Segment3D(const VectorD<3> &start,const VectorD<3> &end);
  //! Get the start=0/end=1 point of the segment
  const VectorD<3>& get_point(unsigned int i) const {
#if IMP_SWIG_WRAPPER
    IMP_USAGE_CHECK(i<2,"invalid point index");
#else
    IMP_INTERNAL_CHECK(i<2, "Invalid point index");
#endif
    return p_[i];
  }
  VectorD<3> get_middle_point() const {
    return .5*p_[0]+ .5*p_[1];
  }
  /** \brief Get a normalized direction vector pointing from
      get_point(0) to get_point(1).
  */
  VectorD<3> get_direction() const {return (p_[1]-p_[0]).get_unit_vector();}
  double get_length() const;
  IMP_SHOWABLE_INLINE({out << spaces_io(p_[0]) << ": " << spaces_io(p_[1]);})
 private:
  VectorD<3> p_[2];
};

IMP_LINEAR_GEOMETRY_METHODS(Segment3D,
                            return BoundingBoxD<3>(g.get_point(0))
                            +BoundingBoxD<3>(g.get_point(1)));



//! Get the distance between a segment and a point
/** \relatesalso Segment3D */
IMPALGEBRAEXPORT double get_distance(const Segment3D &s, const VectorD<3> &p);

//! Get the distance between two segments
/** \relatesalso Segment3D */
IMPALGEBRAEXPORT double get_distance(const Segment3D &a, const Segment3D &b);


IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_SEGMENT_3D_H */
