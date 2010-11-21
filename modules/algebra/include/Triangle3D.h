/**
 *  \file  Triangle3D.h
 *  \brief simple implementation of a triangle in 3D
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */
#ifndef IMPALGEBRA_TRIANGLE_3D_H
#define IMPALGEBRA_TRIANGLE_3D_H

#include "Vector3D.h"
#include "algebra_macros.h"
#include <iostream>
#include <IMP/constants.h>


IMPALGEBRA_BEGIN_NAMESPACE
/**
    \geometry
 */
class IMPALGEBRAEXPORT Triangle3D
{
 public:
  Triangle3D(){}
  Triangle3D(const VectorD<3> &p1,const VectorD<3> &p2,const VectorD<3> &p3);
  //! Get the start=0/end=1 point of the segment
  const VectorD<3>& get_point(unsigned int i) const {
#if defined(IMP_SWIG_WRAPPER)
    IMP_USAGE_CHECK(i<3,"invalid point index");
#else
    IMP_INTERNAL_CHECK(i<2, "Invalid point index");
#endif
    return p_[i];
  }
  IMP_SHOWABLE_INLINE(Triangle3D, out << "(" << p_[0] << ", "
                      << p_[1] << ", " << p_[2] << ")");
  Floats get_edge_lengths() const;
 private:
  VectorD<3> p_[3];
};

IMP_VALUES(Triangle3D, Triangle3Ds);



//! Get bounding triangle
/** \relatesalso Triangle3D */
IMPALGEBRAEXPORT Triangle3D get_bounding_triangle(const Vector3Ds &points);


IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_TRIANGLE_3D_H */
