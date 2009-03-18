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
  //! Get the start/end point of the segment
  Vector3D get_point(unsigned int i) const;
  Vector3D get_middle_point() const {return (start_+get_direction()/2);}
  //! Get the direction of the line
  Vector3D get_direction() const {return end_-start_;}
  //! Get the lenght of the line
  double lenght() const;
  //! Get the distance between a segment and a point
  double distance(const Vector3D &p) const;
  //! Get the projected point on a segment
  Vector3D projection(const Vector3D &p) const;
 private:
  Vector3D start_;
  Vector3D end_;
};

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_SEGMENT_3D_H */
