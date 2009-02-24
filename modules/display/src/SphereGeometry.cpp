/**
 *  \file SphereGeometry.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/display/SphereGeometry.h"


IMPDISPLAY_BEGIN_NAMESPACE

SphereGeometry::SphereGeometry(algebra::Vector3D center,
                               double radius): center_(center),
                                                  radius_(radius){
}

SphereGeometry::~SphereGeometry(){}

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
  return center_;
}
IMPDISPLAY_END_NAMESPACE
