/**
 *  \file PolygonGeometry.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/display/PolygonGeometry.h"


IMPDISPLAY_BEGIN_NAMESPACE

PolygonGeometry::PolygonGeometry(const algebra::Vector3Ds &v): pts_(v){
}

Float PolygonGeometry::get_size() const {
  return 0;
}

algebra::Vector3D PolygonGeometry::get_vertex(unsigned int i) const {
  IMP_check(i < pts_.size(), "Out of range vertex in triangle " << i,
           IndexException);
  return pts_[i];
}

unsigned int PolygonGeometry::get_dimension() const {
  return 2;
}
unsigned int PolygonGeometry::get_number_of_vertices() const{
  return pts_.size();
}
void PolygonGeometry::show(std::ostream &out) const {
  out << "PolygonGeometry: " << pts_.size() << std::endl;
}


IMPDISPLAY_END_NAMESPACE
