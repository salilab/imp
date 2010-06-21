/**
 *  \file QuadraticClosePairsFinder.cpp
 *  \brief Test all pairs.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/core/QuadraticClosePairsFinder.h"
#include "IMP/core/XYZ.h"
#include <IMP/algebra/Sphere3D.h>
#include <IMP/core/utility.h>
#include <cmath>

IMPCORE_BEGIN_NAMESPACE

QuadraticClosePairsFinder::QuadraticClosePairsFinder():
  ClosePairsFinder("QuadraticCPF"){}

ParticlePairsTemp QuadraticClosePairsFinder
::get_close_pairs(SingletonContainer *ca,
                  SingletonContainer *cb) const {
  set_was_used(true);
  IMP_OBJECT_LOG;
  IMP_LOG(TERSE, "Quadratic add_close_pairs called with "
          << ca->get_number_of_particles() << " and "
          << cb->get_number_of_particles() << std::endl);
  ParticlePairsTemp ret;
  ParticlesTemp pta= ca->get_particles();
  ParticlesTemp ptb= cb->get_particles();
  for (unsigned int i=0; i < pta.size(); ++i) {
    for (unsigned int j=0; j < ptb.size(); ++j) {
      if (get_are_close(pta[i], ptb[j])) {
        ret.push_back(ParticlePair(pta[i], ptb[j]));
      }
    }
  }
  return ret;
}

ParticlePairsTemp QuadraticClosePairsFinder
::get_close_pairs(SingletonContainer *c) const {
  set_was_used(true);
  IMP_OBJECT_LOG;
  IMP_LOG(TERSE, "Adding close pairs from "
          << c->get_number_of_particles() << " particles with threshold "
          << get_distance() << std::endl);
  ParticlePairsTemp ret;
  ParticlesTemp pt= c->get_particles();
  for (unsigned int i=0; i< pt.size(); ++i) {
    for (unsigned int j=0; j < i; ++j) {
      if (get_are_close(pt[i], pt[j])) {
        ret.push_back(ParticlePair(pt[i], pt[j]));
      }
    }
  }
  return ret;
}


IntPairs QuadraticClosePairsFinder::
get_close_pairs(const algebra::BoundingBox3Ds &bas,
                const algebra::BoundingBox3Ds &bbs) const {
  set_was_used(true);
  IMP_OBJECT_LOG;
  IMP_LOG(TERSE, "Quadratic add_close_pairs called with "
          << bas.size() << " and "
          << bbs.size() << std::endl);
  IntPairs ret;
  const double d2= get_distance()/2.0;
  for (unsigned int i=0; i< bas.size(); ++i) {
    algebra::BoundingBox3D bi= bas[i]+d2;
    for (unsigned int j=0; j < bbs.size(); ++j) {
      algebra::BoundingBox3D bj= bbs[j]+d2;
      if (get_interiors_intersect(bi, bj)) {
        ret.push_back(IntPair(i,j));
      }
    }
  }
  return ret;
}

IntPairs QuadraticClosePairsFinder::
get_close_pairs(const algebra::BoundingBox3Ds &bbs) const {
  set_was_used(true);
  IMP_OBJECT_LOG;
  IMP_LOG(TERSE, "Adding close pairs from "
          << bbs.size() << " boxes with threshold "
          << get_distance() << std::endl);
  IntPairs ret;
  const double d2= get_distance()/2.0;
  for (unsigned int i=0; i< bbs.size(); ++i) {
    algebra::BoundingBox3D bi= bbs[i]+d2;
    for (unsigned int j=0; j < i; ++j) {
      algebra::BoundingBox3D bj= bbs[j]+d2;
      if (get_interiors_intersect(bi, bj)) {
        ret.push_back(IntPair(i,j));
      }
    }
  }
  return ret;
}


bool QuadraticClosePairsFinder::get_are_close(Particle *a, Particle *b) const {
  XYZ da(a);
  XYZ db(b);
  Float ra= get_radius(a);
  Float rb= get_radius(b);
  Float sr= ra+rb+get_distance();
  for (unsigned int i=0; i< 3; ++i) {
    double delta=std::abs(da.get_coordinate(i) - db.get_coordinate(i));
    if (delta >= sr) {
      return false;
    }
  }
  return get_interiors_intersect(algebra::SphereD<3>(da.get_coordinates(),
                                               ra+get_distance()),
                             algebra::SphereD<3>(db.get_coordinates(), rb));
}


void QuadraticClosePairsFinder::do_show(std::ostream &out) const {
  out << "distance " << get_distance() << std::endl;
}


ParticlesTemp
QuadraticClosePairsFinder::get_input_particles(SingletonContainer *sc) const {
  ParticlesTemp ret= sc->get_particles();
  return ret;
}

ParticlesTemp
QuadraticClosePairsFinder::get_input_particles(SingletonContainer *a,
                                               SingletonContainer *b) const {
  ParticlesTemp ret0= a->get_particles();
  ParticlesTemp ret1= b->get_particles();
  ret0.insert(ret0.end(), ret1.begin(), ret1.end());
  return ret0;
}


ContainersTemp
QuadraticClosePairsFinder::get_input_containers(SingletonContainer *sc) const {
  ContainersTemp ret(1,sc);
  return ret;
}

ContainersTemp
QuadraticClosePairsFinder::get_input_containers(SingletonContainer *a,
                                                SingletonContainer *b) const {
  ContainersTemp ret(2);
  ret[0]= a;
  ret[1]= b;
  return ret;
}

IMPCORE_END_NAMESPACE
