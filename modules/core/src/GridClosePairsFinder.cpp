/**
 *  \file GridClosePairsFinder.cpp
 *  \brief Test all pairs.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/core/GridClosePairsFinder.h"
#include "IMP/core/internal/grid_close_pairs_impl.h"

IMPCORE_BEGIN_NAMESPACE

GridClosePairsFinder::GridClosePairsFinder(): ClosePairsFinder("GridCPF")
{}


ParticlePairsTemp GridClosePairsFinder
::get_close_pairs(const ParticlesTemp &ca,
                  const ParticlesTemp &cb) const {
  IMP_OBJECT_LOG;
  set_was_used(true);
  ParticlePairsTemp out;
  internal::ParticleHelper
    ::fill_close_pairs( internal::ParticleHelper
                        ::get_particle_set(ca.begin(),
                                           ca.end(),0),
                        internal::ParticleHelper
                        ::get_particle_set(cb.begin(),
                                           cb.end(), 1),
                        internal::ParticleTraits(ca[0]->get_model(),
                                                 get_distance()),
                        internal::ParticlePairSink(ca[0]->get_model(),
                                                   access_pair_filters(),
                                                   out));
  return out;
}

ParticlePairsTemp GridClosePairsFinder
::get_close_pairs(const ParticlesTemp &c) const {
  IMP_OBJECT_LOG;
  set_was_used(true);
  IMP_LOG(TERSE, "Rebuilding NBL with Grid and cutoff "
          << get_distance() << std::endl );
  ParticlePairsTemp out;
  internal::ParticleHelper
    ::fill_close_pairs(internal::ParticleHelper::get_particle_set(c.begin(),
                                                                  c.end(),0),
                       internal::ParticleTraits(c[0]->get_model(),
                                                get_distance()),
                       internal::ParticlePairSink(c[0]->get_model(),
                                                  access_pair_filters(),
                                                  out));
  return out;
}

IntPairs GridClosePairsFinder
::get_close_pairs(const algebra::BoundingBox3Ds &bas,
                  const algebra::BoundingBox3Ds &bbs) const {
  IMP_OBJECT_LOG;
  set_was_used(true);
  IntPairs out;
  internal::BBHelper::
    fill_close_pairs(internal::BBHelper::get_particle_set(bas.begin(),
                                                          bas.end(),0),
                     internal::BBHelper::get_particle_set(bbs.begin(),
                                                          bbs.end(),1),
                     internal::BoundingBoxTraits(bas.begin(),
                                                 bbs.begin(), get_distance()),
                     internal::BBPairSink(out));
  return out;
}

IntPairs GridClosePairsFinder
::get_close_pairs(const algebra::BoundingBox3Ds &bas) const {
  IMP_OBJECT_LOG;
  set_was_used(true);
  IntPairs out;
  internal::BBHelper
    ::fill_close_pairs(internal::BBHelper::get_particle_set(bas.begin(),
                                                            bas.end(), 0),
                       internal::BoundingBoxTraits(bas.begin(),
                                                   bas.begin(), get_distance()),
                       internal::BBPairSink(out));
  return out;
}


ParticleIndexPairs GridClosePairsFinder
::get_close_pairs(Model *m,
                  const ParticleIndexes &c) const {
  IMP_OBJECT_LOG;
  set_was_used(true);
  IMP_LOG(TERSE, "Rebuilding NBL with Grid and cutoff "
          << get_distance() << std::endl );
  ParticleIndexPairs out;
  internal::ParticleIndexHelper
    ::fill_close_pairs(internal::ParticleIndexHelper
                       ::get_particle_set(c.begin(),
                                          c.end(),0),
                        internal::ParticleIndexTraits(m,
                                                 get_distance()),
                       internal::ParticleIndexPairSink(m,
                                                       access_pair_filters(),
                                                       out));
  return out;
}
ParticleIndexPairs GridClosePairsFinder
::get_close_pairs(Model *m,
                  const ParticleIndexes &ca,
                  const ParticleIndexes &cb) const {
  IMP_OBJECT_LOG;
  set_was_used(true);
  ParticleIndexPairs out;
  internal::ParticleIndexHelper
    ::fill_close_pairs( internal::ParticleIndexHelper
                        ::get_particle_set(ca.begin(),
                                           ca.end(),0),
                        internal::ParticleIndexHelper
                        ::get_particle_set(cb.begin(),
                                           cb.end(), 1),
                        internal::ParticleIndexTraits(m,
                                                 get_distance()),
                        internal::ParticleIndexPairSink(m,
                                                        access_pair_filters(),
                                                   out));
  return out;
}

void GridClosePairsFinder::do_show(std::ostream &out) const {
  out << "distance " << get_distance() << std::endl;
}


ParticlesTemp
GridClosePairsFinder::get_input_particles(const ParticlesTemp &ps) const {
  ParticlesTemp ret=ps;
  if (get_number_of_pair_filters() >0) {
    ParticlesTemp retc;
    for (PairFilterConstIterator it= pair_filters_begin();
         it != pair_filters_end(); ++it) {
      for (unsigned int i=0; i< ret.size(); ++i) {
        ParticlesTemp cur= (*it)->get_input_particles(ret[i]);
        retc.insert(retc.end(), cur.begin(), cur.end());
      }
    }
    ret.insert(ret.end(), retc.begin(), retc.end());
  }
  return ret;
}

ContainersTemp
GridClosePairsFinder::get_input_containers(const ParticlesTemp &) const {
  return ContainersTemp();
}

IMPCORE_END_NAMESPACE
