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
                               double radius,
                               Color color,
                               std::string name): center_(center),
                                                  radius_(radius),
                                                  name_(name){
  if (color != Color()) set_color(color);
}

SphereGeometry::~SphereGeometry(){}

void SphereGeometry::show(std::ostream &out) const {
  out << "SphereGeometry" << std::endl;
}

unsigned int SphereGeometry::get_dimension() const{
  return 0;
}
algebra::Vector3D SphereGeometry::get_vertex(unsigned int) const {
  return center_;
}
IMPDISPLAY_END_NAMESPACE
