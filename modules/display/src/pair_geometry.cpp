/**
 *  \file pair_geometry.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/display/pair_geometry.h"
#include <IMP/core/XYZ.h>
#include <IMP/display/CylinderGeometry.h>


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
                             Float radius): c_(pc),
                                            radius_(radius){
  threshold_=0;
}

PairsGeometry::PairsGeometry(PairScore *f,
                             PairContainer *c): f_(f), c_(c) {
  threshold_=0;
  radius_=1;
}

void PairsGeometry::show(std::ostream &out) const {
  out << "PairsGeometry" << std::endl;
}

Geometries PairsGeometry::get_geometry() const {
  std::map<Particle*, algebra::Vector3D> base_derivatives;
  if (f_) {
    for (PairContainer::ParticlePairIterator pit = c_->particle_pairs_begin();
         pit != c_->particle_pairs_end(); ++pit) {
      core::XYZ d0((*pit)[0]), d1((*pit)[1]);
      base_derivatives[d0]= d0.get_derivatives();
      base_derivatives[d1]= d1.get_derivatives();
    }
  }
  DerivativeAccumulator da;
  Geometries ret;
  for (PairContainer::ParticlePairIterator pit = c_->particle_pairs_begin();
       pit != c_->particle_pairs_end(); ++pit) {
    if (!f_ || f_->evaluate((*pit)[0], (*pit)[1], &da) > threshold_) {
      core::XYZ d0((*pit)[0]), d1((*pit)[1]);
      IMP_NEW(CylinderGeometry, c, (algebra::Cylinder3D(
                               algebra::Segment3D(d0.get_coordinates(),
                                                  d1.get_coordinates()),
                               radius_)));
      c->set_color(get_color());
      c->set_name(get_name());
      ret.push_back(c);
    }
  }
  if (f_) {
    for (std::map<Particle*, algebra::Vector3D>::iterator
           it= base_derivatives.begin();
         it != base_derivatives.end(); ++it) {
      core::XYZ d(it->first);
      algebra::Vector3D deriv= d.get_derivatives()- it->second;
      IMP_NEW(CylinderGeometry, c,
              (algebra::Cylinder3D(algebra::Segment3D(d.get_coordinates(),
                                             d.get_coordinates()+deriv),
                                                        radius_)));
      c->set_color(.5*get_color());
      c->set_name(get_name()+" derivative");
      ret.push_back(c);
    }
  }
  return ret;
}

IMPDISPLAY_END_NAMESPACE
