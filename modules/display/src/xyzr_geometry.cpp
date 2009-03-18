/**
 *  \file xyzr_geometry.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/display/xyzr_geometry.h"


IMPDISPLAY_BEGIN_NAMESPACE

XYZRGeometry::XYZRGeometry(core::XYZRDecorator d): d_(d){
}

XYZRGeometry::~XYZRGeometry(){}

void XYZRGeometry::show(std::ostream &out) const {
  out << "XYZRGeometry" << std::endl;
}

unsigned int XYZRGeometry::get_dimension() const{
  return 0;
}
unsigned int XYZRGeometry::get_number_of_vertices() const{
  return 1;
}
algebra::Vector3D XYZRGeometry::get_vertex(unsigned int i) const {
  return d_.get_coordinates();
}

Float XYZRGeometry::get_size() const {
  return d_.get_radius();
}


IMPDISPLAY_END_NAMESPACE
