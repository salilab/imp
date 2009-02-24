/**
 *  \file CylinderGeometry.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/display/CylinderGeometry.h"


IMPDISPLAY_BEGIN_NAMESPACE

CylinderGeometry::CylinderGeometry(const algebra::Vector3D& p0,
                                   const algebra::Vector3D& p1,
                                   double radius): radius_(radius){
  p_[0]= p0;
  p_[1]= p1;
}

CylinderGeometry::~CylinderGeometry(){}

void CylinderGeometry::show(std::ostream &out) const {
  out << "CylinderGeometry" << std::endl;
}

unsigned int CylinderGeometry::get_dimension() const{
  return 1;
}
algebra::Vector3D CylinderGeometry::get_vertex(unsigned int i) const {
  IMP_assert(i < 2, "Invalid vertex of cylinder");
  return p_[i];
}
IMPDISPLAY_END_NAMESPACE
