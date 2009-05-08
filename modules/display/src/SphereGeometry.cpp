/**
 *  \file SphereGeometry.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/display/SphereGeometry.h"


IMPDISPLAY_BEGIN_NAMESPACE

SphereGeometry::SphereGeometry(const algebra::Sphere3D &s): s_(s){
}

void SphereGeometry::show(std::ostream &out) const {
  out << "SphereGeometry" << std::endl;
}

unsigned int SphereGeometry::get_dimension() const{
  return 0;
}
unsigned int SphereGeometry::get_number_of_vertices() const{
  return 1;
}
algebra::Vector3D SphereGeometry::get_vertex(unsigned int) const {
  return s_.get_center();
}


Float SphereGeometry::get_size() const {
  return s_.get_radius();
}

IMPDISPLAY_END_NAMESPACE
