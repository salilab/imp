/**
 *  \file QuadraticClosePairsFinder.cpp
 *  \brief Test all pairs.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/core/QuadraticClosePairsFinder.h"
#include "IMP/core/internal/close_pairs_helpers.h"
#include "IMP/core/XYZ.h"
#include <IMP/algebra/Sphere3D.h>
#include <IMP/core/utility.h>
#include <cmath>

IMPCORE_BEGIN_NAMESPACE

QuadraticClosePairsFinder::QuadraticClosePairsFinder():
  ClosePairsFinder("QuadraticCPF"){}

ParticlePairsTemp QuadraticClosePairsFinder
::get_close_pairs(const ParticlesTemp &pta,
                  const ParticlesTemp &ptb) const {
  set_was_used(true);
  IMP_OBJECT_LOG;
  IMP_LOG_TERSE( "Quadratic add_close_pairs called with "
          << pta.size() << " and "
          << ptb.size() << std::endl);
  ParticlePairsTemp ret;
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
::get_close_pairs(const ParticlesTemp &pt) const {
  set_was_used(true);
  IMP_OBJECT_LOG;
  IMP_LOG_TERSE( "Adding close pairs from "
          << pt.size() << " particles with threshold "
          << get_distance() << std::endl);
  ParticlePairsTemp ret;
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
  IMP_LOG_TERSE( "Quadratic add_close_pairs called with "
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
  IMP_LOG_TERSE( "Adding close pairs from "
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
  return internal::get_are_close(a->get_model(), access_pair_filters(),
                                 a->get_index(),
                                 b->get_index(), get_distance());
}


void QuadraticClosePairsFinder::do_show(std::ostream &out) const {
  out << "distance " << get_distance() << std::endl;
}



ModelObjectsTemp
QuadraticClosePairsFinder::do_get_inputs(Model *m,
                                    const ParticleIndexes &pis) const {
  ModelObjectsTemp ret;
  ret+=IMP::get_particles(m, pis);
  for (PairFilterConstIterator it= pair_filters_begin();
       it != pair_filters_end(); ++it) {
    ret+=(*it)->get_inputs(m, pis);
  }
  return ret;
}

IMPCORE_END_NAMESPACE
