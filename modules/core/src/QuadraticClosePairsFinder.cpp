/**
 *  \file QuadraticClosePairsFinder.cpp
 *  \brief Test all pairs.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/core/QuadraticClosePairsFinder.h"
#include "IMP/core/internal/close_pairs_helpers.h"
#include "IMP/core/XYZ.h"
#include <IMP/algebra/Sphere3D.h>
#include <IMP/core/utility.h>
#include <cmath>

IMPCORE_BEGIN_NAMESPACE

QuadraticClosePairsFinder::QuadraticClosePairsFinder()
    : ClosePairsFinder("QuadraticCPF") {}

ParticleIndexPairs QuadraticClosePairsFinder::get_close_pairs(
    Model *m, const ParticleIndexes &pta,
    const ParticleIndexes &ptb) const {
  set_was_used(true);
  IMP_OBJECT_LOG;
  IMP_LOG_TERSE("Quadratic add_close_pairs called with "
                << pta.size() << " and " << ptb.size() << std::endl);
  ParticleIndexPairs ret;
  for (unsigned int i = 0; i < pta.size(); ++i) {
    for (unsigned int j = 0; j < ptb.size(); ++j) {
      if (get_are_close_and_filtered(m, pta[i], ptb[j])) {
        ret.push_back(ParticleIndexPair(pta[i], ptb[j]));
      }
    }
  }
  return ret;
}

ParticleIndexPairs QuadraticClosePairsFinder::get_close_pairs(
    Model *m, const ParticleIndexes &pt) const {
  set_was_used(true);
  IMP_OBJECT_LOG;
  IMP_LOG_TERSE("Adding close pairs from "
                << pt.size() << " particles with threshold " << get_distance()
                << std::endl);
  ParticleIndexPairs ret;
  for (unsigned int i = 0; i < pt.size(); ++i) {
    for (unsigned int j = 0; j < i; ++j) {
      if (get_are_close_and_filtered(m, pt[i], pt[j])) {
        ret.push_back(ParticleIndexPair(pt[i], pt[j]));
      }
    }
  }
  return ret;
}

IntPairs QuadraticClosePairsFinder::get_close_pairs(
    const algebra::BoundingBox3Ds &bas,
    const algebra::BoundingBox3Ds &bbs) const {
  set_was_used(true);
  IMP_OBJECT_LOG;
  IMP_LOG_TERSE("Quadratic add_close_pairs called with "
                << bas.size() << " and " << bbs.size() << std::endl);
  IntPairs ret;
  const double d2 = get_distance() / 2.0;
  for (unsigned int i = 0; i < bas.size(); ++i) {
    algebra::BoundingBox3D bi = bas[i] + d2;
    for (unsigned int j = 0; j < bbs.size(); ++j) {
      algebra::BoundingBox3D bj = bbs[j] + d2;
      if (get_interiors_intersect(bi, bj)) {
        ret.push_back(IntPair(i, j));
      }
    }
  }
  return ret;
}

IntPairs QuadraticClosePairsFinder::get_close_pairs(
    const algebra::BoundingBox3Ds &bbs) const {
  set_was_used(true);
  IMP_OBJECT_LOG;
  IMP_LOG_TERSE("Adding close pairs from "
                << bbs.size() << " boxes with threshold " << get_distance()
                << std::endl);
  IntPairs ret;
  const double d2 = get_distance() / 2.0;
  for (unsigned int i = 0; i < bbs.size(); ++i) {
    algebra::BoundingBox3D bi = bbs[i] + d2;
    for (unsigned int j = 0; j < i; ++j) {
      algebra::BoundingBox3D bj = bbs[j] + d2;
      if (get_interiors_intersect(bi, bj)) {
        ret.push_back(IntPair(i, j));
      }
    }
  }
  return ret;
}

bool QuadraticClosePairsFinder::get_are_close_and_filtered
(Model *m,
 ParticleIndex a,
 ParticleIndex b) const
{
  return internal::get_are_close_and_filtered
    (m, access_pair_filters(),
     a, b,
     get_distance());
}

ModelObjectsTemp QuadraticClosePairsFinder::do_get_inputs(
    Model *m, const ParticleIndexes &pis) const {
  ModelObjectsTemp ret;
  ret += IMP::get_particles(m, pis);
  for (PairFilterConstIterator it = pair_filters_begin();
       it != pair_filters_end(); ++it) {
    ret += (*it)->get_inputs(m, pis);
  }
  return ret;
}

IMPCORE_END_NAMESPACE
