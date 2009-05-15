/**
 *  \file pair_geometry.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/display/pair_geometry.h"
#include <IMP/core/XYZ.h>


IMPDISPLAY_BEGIN_NAMESPACE

PairGeometry::PairGeometry(core::XYZ p0,
                           core::XYZ p1,
                           Float radius): d0_(p0), d1_(p1),
                                          radius_(radius){
}

void PairGeometry::show(std::ostream &out) const {
  out << "PairGeometry" << std::endl;
}

unsigned int PairGeometry::get_dimension() const{
  return 1;
}
algebra::Vector3D PairGeometry::get_vertex(unsigned int i) const {
  if (i==0) return d0_.get_coordinates();
  else return d1_.get_coordinates();
}

Float PairGeometry::get_size() const {
  return radius_;
}
unsigned int PairGeometry::get_number_of_vertices() const{
  return 2;
}

PairsGeometry::PairsGeometry(PairContainer *pc,
                             Float radius): pc_(pc),
                                            radius_(radius){
}

void PairsGeometry::show(std::ostream &out) const {
  out << "PairsGeometry" << std::endl;
}

Geometries PairsGeometry::get_geometry() const {
  Geometries ret;
  for (PairContainer::ParticlePairIterator it= pc_->particle_pairs_begin();
       it != pc_->particle_pairs_end(); ++it) {
    ret.push_back(new PairGeometry(core::XYZ(it->first),
                                   core::XYZ(it->second), radius_));
  }
  return ret;
}

IMPDISPLAY_END_NAMESPACE
