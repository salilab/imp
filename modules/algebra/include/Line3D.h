/**
 *  \file  Line3D.h
 *  \brief stores a line in 3D
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */
#ifndef IMPALGEBRA_LINE_3D_H
#define IMPALGEBRA_LINE_3D_H

#include <IMP/algebra/Vector3D.h>
#include <iostream>
#include <IMP/constants.h>


IMPALGEBRA_BEGIN_NAMESPACE
//! Line3D
class IMPALGEBRAEXPORT Line3D
{
 public:
  Line3D(){}
  Line3D(const Vector3D &start,const Vector3D &end);
  //! Get the starting point of the line
  Vector3D get_start_point() const {return start_;}
  //! Get the end point of the line
  Vector3D get_end_point() const {return end_;}
  //! Get the direction of the line
  Vector3D get_direction() const {return end_-start_;}
  //! Get the lenght of the line
  double lenght() const;
 private:
  Vector3D start_;
  Vector3D end_;
};

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_LINE_3D_H */
