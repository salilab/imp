/**
 *  \file bond_geometry.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/display/bond_geometry.h"
#include <IMP/core/XYZ.h>


IMPDISPLAY_BEGIN_NAMESPACE

BondGeometry::BondGeometry(atom::Bond p,
                           Float radius): d_(p),
                                          radius_(radius){
  set_name(p.get_particle()->get_name());
}

void BondGeometry::show(std::ostream &out) const {
  out << "BondGeometry" << std::endl;
}

unsigned int BondGeometry::get_dimension() const{
  return 1;
}
algebra::Vector3D BondGeometry::get_vertex(unsigned int i) const {
  atom::Bonded ep=  d_.get_bonded(i);
  core::XYZ epi(ep.get_particle());
  return epi.get_coordinates();
}

Float BondGeometry::get_size() const {
  return radius_;
}
unsigned int BondGeometry::get_number_of_vertices() const{
  return 2;
}





BondsGeometry::BondsGeometry(SingletonContainer *sc,
                             FloatKey rk): sc_(sc), rk_(rk){}

void BondsGeometry::show(std::ostream &out) const {
  out << "Bondss" << std::endl;
}

Geometries BondsGeometry::get_geometry() const {
  Geometries ret(sc_->get_number_of_particles());
  for (unsigned int i=0; i< ret.size(); ++i) {
    double r= sc_->get_particle(i)->get_value(rk_);
    IMP_NEW(BondGeometry, g, (atom::Bond(sc_->get_particle(i)), r));
    if (!get_name().empty()) g->set_name(get_name());
    g->set_color(get_color());
    ret.set(i, g);
  }
  return ret;
}


IMPDISPLAY_END_NAMESPACE
