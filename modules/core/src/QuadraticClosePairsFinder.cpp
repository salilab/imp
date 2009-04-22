/**
 *  \file QuadraticClosePairsFinder.cpp
 *  \brief Test all pairs.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/core/QuadraticClosePairsFinder.h"
#include "IMP/core/XYZDecorator.h"
#include <IMP/algebra/Sphere3D.h>
#include <cmath>

IMPCORE_BEGIN_NAMESPACE

QuadraticClosePairsFinder::QuadraticClosePairsFinder(){}

QuadraticClosePairsFinder::~QuadraticClosePairsFinder(){}

void QuadraticClosePairsFinder
::add_close_pairs(SingletonContainer *ca,
                  SingletonContainer *cb,
                  FilteredListPairContainer *out) const {
  IMP_LOG(TERSE, "Quadratic add_close_pairs called with "
          << ca->get_number_of_particles() << " and "
          << cb->get_number_of_particles() << std::endl);
  for (SingletonContainer::ParticleIterator it = ca->particles_begin();
       it != ca->particles_end(); ++it) {
    for (SingletonContainer::ParticleIterator it2 = cb->particles_begin();
         it2 != cb->particles_end(); ++it2) {
      if (get_are_close(*it, *it2)) {
        out->add_particle_pair(ParticlePair(*it, *it2));
      }
    }
  }
}

void QuadraticClosePairsFinder
::add_close_pairs(SingletonContainer *c,
                  FilteredListPairContainer *out) const {
  IMP_LOG(TERSE, "Adding close pairs from "
          << c->get_number_of_particles() << " particles." << std::endl);
  for (SingletonContainer::ParticleIterator it = c->particles_begin();
       it != c->particles_end(); ++it) {
    for (SingletonContainer::ParticleIterator it2 = c->particles_begin();
       it2 != it; ++it2) {
      if (get_are_close(*it, *it2)) {
        out->add_particle_pair(ParticlePair(*it, *it2));
      }
    }
  }
}


bool QuadraticClosePairsFinder::get_are_close(Particle *a, Particle *b) const {
  XYZDecorator da(a);
  XYZDecorator db(b);
  Float ra= get_radius(a);
  Float rb= get_radius(b);
  Float sr= ra+rb+get_distance();
  for (unsigned int i=0; i< 3; ++i) {
    double delta=std::abs(da.get_coordinate(i) - db.get_coordinate(i));
    if (delta >= sr) {
      return false;
    }
  }
  return interiors_intersect(algebra::Sphere3D(da.get_coordinates(),
                                               ra+get_distance()),
                             algebra::Sphere3D(db.get_coordinates(), rb));
}

IMPCORE_END_NAMESPACE
