/**
 *  \file QuadraticClosePairsFinder.cpp
 *  \brief Test all pairs.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/core/QuadraticClosePairsFinder.h"
#include "IMP/core/XYZDecorator.h"

IMPCORE_BEGIN_NAMESPACE

QuadraticClosePairsFinder::QuadraticClosePairsFinder(){}

QuadraticClosePairsFinder::~QuadraticClosePairsFinder(){}

void QuadraticClosePairsFinder
::add_close_pairs(ParticleContainer *ca,
                  ParticleContainer *cb,
                  FilteredListParticlePairContainer *out) {
  for (ParticleContainer::ParticleIterator it = ca->particles_begin();
       it != ca->particles_end(); ++it) {
    for (ParticleContainer::ParticleIterator it2 = cb->particles_begin();
         it2 != cb->particles_end(); ++it2) {
      if (get_are_close(*it, *it2)) {
        out->add_particle_pair(ParticlePair(*it, *it2));
      }
    }
  }
}

void QuadraticClosePairsFinder
::add_close_pairs(ParticleContainer *c,
                  FilteredListParticlePairContainer *out) {
  for (ParticleContainer::ParticleIterator it = c->particles_begin();
       it != c->particles_end(); ++it) {
    for (ParticleContainer::ParticleIterator it2 = c->particles_begin();
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
  Float sr= ra+rb;
  for (unsigned int i=0; i< 3; ++i) {
    float delta=std::abs(da.get_coordinate(i) - db.get_coordinate(i));
    if (delta - sr >= get_distance()) {
      return false;
    }
  }
  return true;
}

IMPCORE_END_NAMESPACE
