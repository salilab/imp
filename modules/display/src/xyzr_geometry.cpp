/**
 *  \file xyzr_geometry.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/display/xyzr_geometry.h"
#include <IMP/display/Colored.h>

IMPDISPLAY_BEGIN_NAMESPACE

XYZRGeometry::XYZRGeometry(core::XYZR d): d_(d){
  set_name(d.get_particle()->get_name());
}

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

Color XYZRGeometry::get_color() const {
  if (Colored::particle_is_instance(d_)) return Colored(d_).get_color();
  else return Geometry::get_color();
}

XYZRsGeometry::XYZRsGeometry(SingletonContainer *sc,
                             FloatKey rk): sc_(sc), rk_(rk){}

void XYZRsGeometry::show(std::ostream &out) const {
  out << "XYZRs" << std::endl;
}

Geometries XYZRsGeometry::get_geometry() const {
  Geometries ret(sc_->get_number_of_particles());
  for (unsigned int i=0; i< ret.size(); ++i) {
    IMP_NEW( XYZRGeometry, g, (core::XYZR(sc_->get_particle(i))));
    if (!get_name().empty()) g->set_name(get_name());
    g->set_color(get_color());
    ret.set(i, g);
  }
  return ret;
}


IMPDISPLAY_END_NAMESPACE
