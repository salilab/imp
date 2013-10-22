/**
 *  \file GridClosePairsFinder.cpp
 *  \brief Test all pairs.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/core/GridClosePairsFinder.h"
#include "IMP/core/internal/grid_close_pairs_impl.h"

IMPCORE_BEGIN_NAMESPACE

GridClosePairsFinder::GridClosePairsFinder() : ClosePairsFinder("GridCPF") {}

IntPairs GridClosePairsFinder::get_close_pairs(
    const algebra::BoundingBox3Ds &bas,
    const algebra::BoundingBox3Ds &bbs) const {
  IMP_OBJECT_LOG;
  set_was_used(true);
  IntPairs out;
  internal::BBHelper::fill_close_pairs(
      internal::BBHelper::get_particle_set(bas.begin(), bas.end(), 0),
      internal::BBHelper::get_particle_set(bbs.begin(), bbs.end(), 1),
      internal::BoundingBoxTraits(bas.begin(), bbs.begin(), get_distance()),
      internal::BBPairSink(out));
  return out;
}

IntPairs GridClosePairsFinder::get_close_pairs(
    const algebra::BoundingBox3Ds &bas) const {
  IMP_OBJECT_LOG;
  set_was_used(true);
  IntPairs out;
  internal::BBHelper::fill_close_pairs(
      internal::BBHelper::get_particle_set(bas.begin(), bas.end(), 0),
      internal::BoundingBoxTraits(bas.begin(), bas.begin(), get_distance()),
      internal::BBPairSink(out));
  return out;
}

ParticleIndexPairs GridClosePairsFinder::get_close_pairs(
    kernel::Model *m, const kernel::ParticleIndexes &c) const {
  IMP_OBJECT_LOG;
  set_was_used(true);
  IMP_LOG_TERSE("Rebuilding NBL with Grid and cutoff " << get_distance()
                                                       << std::endl);
  kernel::ParticleIndexPairs out;
  internal::ParticleIndexHelper::fill_close_pairs(
      internal::ParticleIndexHelper::get_particle_set(c.begin(), c.end(), 0),
      internal::ParticleIndexTraits(m, get_distance()),
      internal::ParticleIndexPairSink(m, access_pair_filters(), out));
  return out;
}
ParticleIndexPairs GridClosePairsFinder::get_close_pairs(
    kernel::Model *m, const kernel::ParticleIndexes &ca,
    const kernel::ParticleIndexes &cb) const {
  IMP_OBJECT_LOG;
  set_was_used(true);
  kernel::ParticleIndexPairs out;
  internal::ParticleIndexHelper::fill_close_pairs(
      internal::ParticleIndexHelper::get_particle_set(ca.begin(), ca.end(), 0),
      internal::ParticleIndexHelper::get_particle_set(cb.begin(), cb.end(), 1),
      internal::ParticleIndexTraits(m, get_distance()),
      internal::ParticleIndexPairSink(m, access_pair_filters(), out));
  return out;
}

ModelObjectsTemp GridClosePairsFinder::do_get_inputs(
    kernel::Model *m, const kernel::ParticleIndexes &pis) const {
  kernel::ModelObjectsTemp ret;
  ret += IMP::get_particles(m, pis);
  for (PairFilterConstIterator it = pair_filters_begin();
       it != pair_filters_end(); ++it) {
    ret += (*it)->get_inputs(m, pis);
  }
  return ret;
}

IMPCORE_END_NAMESPACE
