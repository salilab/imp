/**
 *  \file  Segment3D.h
 *  \brief simple implementation of segments in 3D
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */
#ifndef IMPALGEBRA_SEGMENT_3D_H
#define IMPALGEBRA_SEGMENT_3D_H

#include "Vector3D.h"
#include <iostream>
#include <IMP/constants.h>


IMPALGEBRA_BEGIN_NAMESPACE
/** */
class IMPALGEBRAEXPORT Segment3D: public UninitializedDefault
{
 public:
  Segment3D(){}
  Segment3D(const Vector3D &start,const Vector3D &end);
  //! Get the start=0/end=1 point of the segment
  const Vector3D& get_point(unsigned int i) const {
#if IMP_SWIG_WRAPPER
    IMP_check(i<2,"invalid point index",ValueException);
#else
    IMP_assert(i<2, "Invalid point index");
#endif
    return p_[i];
  }
  Vector3D get_middle_point() const {return (p_[0]+p_[1])/2.0;}
  /** \brief Get a normalized direction vector pointing from
      get_point(0) to get_point(1).
  */
  Vector3D get_direction() const {return (p_[1]-p_[0]).get_unit_vector();}
  double get_length() const;
  void show(std::ostream &out=std::cout) const {
    out << spaces_io(p_[0]) << ": " << spaces_io(p_[1]);
  }
 private:
  Vector3D p_[2];
};

IMP_OUTPUT_OPERATOR(Segment3D)


//! Get the distance between a segment and a point
/** \relates Segment3D */
IMPALGEBRAEXPORT double distance(const Segment3D &s, const Vector3D &p);

//! Get the distance between two segments
/** \relates Segment3D */
IMPALGEBRAEXPORT double distance(const Segment3D &a, const Segment3D &b);

//! Get the projected point on a segment
/** It doesn't really make sense for a segment. I vote it goes away.
    This produces the wrong answer for points that don't project between
    the endpoints.
    \relates Segment3D
 */
IMPALGEBRAEXPORT Vector3D projection(const Segment3D &s, const Vector3D &p);

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_SEGMENT_3D_H */
