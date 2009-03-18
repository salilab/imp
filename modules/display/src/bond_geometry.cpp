/**
 *  \file bond_geometry.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/display/bond_geometry.h"
#include <IMP/core/XYZDecorator.h>


IMPDISPLAY_BEGIN_NAMESPACE

BondGeometry::BondGeometry(atom::BondDecorator p,
                           Float radius): d_(p),
                                          radius_(radius){
}

BondGeometry::~BondGeometry(){}

void BondGeometry::show(std::ostream &out) const {
  out << "BondGeometry" << std::endl;
}

unsigned int BondGeometry::get_dimension() const{
  return 1;
}
algebra::Vector3D BondGeometry::get_vertex(unsigned int i) const {
  atom::BondedDecorator ep=  d_.get_bonded(i);
  core::XYZDecorator epi(ep.get_particle());
  return epi.get_coordinates();
}

Float BondGeometry::get_size() const {
  return radius_;
}
unsigned int BondGeometry::get_number_of_vertices() const{
  return 2;
}
IMPDISPLAY_END_NAMESPACE
