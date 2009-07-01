/**
 *  \file CylinderGeometry.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/display/CylinderGeometry.h"


IMPDISPLAY_BEGIN_NAMESPACE

CylinderGeometry::CylinderGeometry(const algebra::Cylinder3D &c): c_(c){
}

void CylinderGeometry::show(std::ostream &out) const {
  out << "CylinderGeometry" << std::endl;
}

unsigned int CylinderGeometry::get_dimension() const{
  return 1;
}
unsigned int CylinderGeometry::get_number_of_vertices() const{
  return 2;
}
algebra::Vector3D CylinderGeometry::get_vertex(unsigned int i) const {
  return c_.get_segment().get_point(i);
}


Float CylinderGeometry::get_size() const {
  return c_.get_radius();
}



PolyCylinderGeometry::PolyCylinderGeometry(const algebra::Vector3Ds &c,
                                           double r): c_(c), r_(r) {
}

void PolyCylinderGeometry::show(std::ostream &out) const {
  out << "CylinderGeometry" << std::endl;
}

unsigned int PolyCylinderGeometry::get_dimension() const{
  return 1;
}
unsigned int PolyCylinderGeometry::get_number_of_vertices() const{
  return c_.size();
}
algebra::Vector3D PolyCylinderGeometry::get_vertex(unsigned int i) const {
  return c_[i];
}


Float PolyCylinderGeometry::get_size() const {
  return r_;
}
IMPDISPLAY_END_NAMESPACE
