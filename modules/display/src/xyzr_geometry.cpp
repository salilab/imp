/**
 *  \file XYZRGeometry.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
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
algebra::Vector3D XYZRGeometry::get_vertex(unsigned int i) const {
  return d_.get_coordinates();
}

Float XYZRGeometry::get_size() const {
  return d_.get_radius();
}


XYZRGeometryExtractor::XYZRGeometryExtractor(FloatKey rk): rk_(rk){
}

XYZRGeometryExtractor::~XYZRGeometryExtractor(){}

void XYZRGeometryExtractor::show(std::ostream &out) const {
  out << "XYZRGeometryExtractor" << std::endl;
}

Geometries XYZRGeometryExtractor::get_geometries(Particle *p) const {
  return Geometries(1, new XYZRGeometry(core::XYZRDecorator(p, rk_)));
}

IMPDISPLAY_END_NAMESPACE
