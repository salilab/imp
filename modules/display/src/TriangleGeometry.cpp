/**
 *  \file TriangleGeometry.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/display/TriangleGeometry.h"


IMPDISPLAY_BEGIN_NAMESPACE

TriangleGeometry::TriangleGeometry(const algebra::Vector3D &a,
                                   const algebra::Vector3D &b,
                                   const algebra::Vector3D &c){
  v_[0]=a;
  v_[1]=b;
  v_[2]=c;
}

TriangleGeometry::~TriangleGeometry(){}

algebra::Vector3D TriangleGeometry::get_vertex(unsigned int i) const {
  IMP_check(i < 3, "Out of range vertex in triangle " << i,
           IndexException);
  return v_[i];
}

unsigned int TriangleGeometry::get_dimension() const {
  return 2;
}
unsigned int TriangleGeometry::get_number_of_vertices() const{
  return 3;
}
void TriangleGeometry::show(std::ostream &out) const {
  out << "TriangleGeometry: " << v_[0] << " " << v_[1]
      << " " << v_[2] << std::endl;
}

IMPDISPLAY_END_NAMESPACE
