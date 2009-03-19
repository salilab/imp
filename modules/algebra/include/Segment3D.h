/**
 *  \file  Segment3D.h
 *  \brief simple implementation of segments in 3D
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */
#ifndef IMPALGEBRA_SEGMENT_3D_H
#define IMPALGEBRA_SEGMENT_3D_H

#include <IMP/algebra/Vector3D.h>
#include <iostream>
#include <IMP/constants.h>


IMPALGEBRA_BEGIN_NAMESPACE
//! Segment3D
class IMPALGEBRAEXPORT Segment3D
{
 public:
  Segment3D(){}
  Segment3D(const Vector3D &start,const Vector3D &end);
  //! Get the start=0/end=1 point of the segment
  Vector3D get_point(unsigned int i) const {
#if IMP_SWIG_WRAPPER
    IMP_check(i<2,"invalid point index",ValueException);
#else
    IMP_assert(i<2, "Invalid point index");
#endif
    return p_[i];
  }
  Vector3D get_middle_point() const {return (p_[0]+get_direction()/2);}
  //! Get the unnormalized direction of the segment
  Vector3D get_direction() const {return p_[1]-p_[0];}
  //! Get the lenght of the line
  double get_length() const;
  void show(std::ostream &out=std::cout) const {
    out << spaces_io(p_[0]) << ": " << spaces_io(p_[1]);
  }
 private:
  Vector3D p_[2];
};

IMP_OUTPUT_OPERATOR(Segment3D)


//! Get the distance between a segment and a point
IMPALGEBRAEXPORT double distance(const Segment3D &s, const Vector3D &p);

//! Get the distance between two segments
IMPALGEBRAEXPORT double distance(const Segment3D &a, const Segment3D &b);

//! Get the projected point on a segment
/** It doesn't really make sense for a segment. I vote it goes away.
    This produces the wrong answer for points that don't project between
    the endpoints.
 */
IMPALGEBRAEXPORT Vector3D projection(const Segment3D &s, const Vector3D &p);

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_SEGMENT_3D_H */
