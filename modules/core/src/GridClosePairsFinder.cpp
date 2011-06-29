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

GridClosePairsFinder::GridClosePairsFinder(): ClosePairsFinder("GridCPF"),
                                              merged_(0)
{}

GridClosePairsFinder::GridClosePairsFinder(const algebra::BoundingBox3D &bb,
                                           unsigned int merged_boundaries):
  ClosePairsFinder("PeriodicGridCPF"), bb_(bb), merged_(merged_boundaries) {}


ParticlePairsTemp GridClosePairsFinder
::get_close_pairs(const ParticlesTemp &ca,
                  const ParticlesTemp &cb) const {
  IMP_OBJECT_LOG;
  set_was_used(true);
  ParticlePairsTemp out;
  if (merged_) {
    internal::PParticleHelper
      ::fill_close_pairs(internal::PParticleHelper
                         ::get_particle_set(ca.begin(),
                                            ca.end(),
                                            internal::ParticleID(),
                                            internal::ParticleCenter(),
                                            internal::ParticleRadius()),
                         internal::PParticleHelper
                         ::get_particle_set(cb.begin(),
                                            cb.end(),
                                            internal::ParticleID(),
                                            internal::ParticleCenter(),
                                            internal::ParticleRadius()),
                         internal::PeriodicParticleClose(bb_, merged_,
                                               get_distance()),
                         get_distance(), bb_, merged_,
                         internal::ParticlePairSink(out));
  } else {
     internal::ParticleHelper
      ::fill_close_pairs( internal::ParticleHelper
                         ::get_particle_set(ca.begin(),
                                            ca.end(),
                                            internal::ParticleID(),
                                            internal::ParticleCenter(),
                                            internal::ParticleRadius()),
                          internal::ParticleHelper
                         ::get_particle_set(cb.begin(),
                                            cb.end(),
                                            internal::ParticleID(),
                                            internal::ParticleCenter(),
                                            internal::ParticleRadius()),
                          internal::ParticleClose(get_distance()),
                         get_distance(), bb_, merged_,
                          internal::ParticlePairSink(out));
  }
  return out;
}

ParticlePairsTemp GridClosePairsFinder
::get_close_pairs(const ParticlesTemp &c) const {
  IMP_OBJECT_LOG;
  set_was_used(true);
  IMP_LOG(TERSE, "Rebuilding NBL with Grid and cutoff "
          << get_distance() << std::endl );
  ParticlePairsTemp out;
  if (merged_) {
     internal::PParticleHelper
      ::fill_close_pairs(internal::PParticleHelper::get_particle_set(c.begin(),
                                                           c.end(),
                                                     internal::ParticleID(),
                                                  internal::ParticleCenter(),
                                                  internal::ParticleRadius()),
                 internal::PeriodicParticleClose(bb_, merged_, get_distance()),
                         get_distance(), bb_, merged_,
                         internal::ParticlePairSink(out));
  } else {
     internal::ParticleHelper
      ::fill_close_pairs(internal::ParticleHelper::get_particle_set(c.begin(),
                                                                    c.end(),
                                                        internal::ParticleID(),
                                                    internal::ParticleCenter(),
                                                   internal::ParticleRadius()),
                         internal::ParticleClose(get_distance()),
                         get_distance(), bb_, merged_,
                         internal::ParticlePairSink(out));
  }
  return out;
}

IntPairs GridClosePairsFinder
::get_close_pairs(const algebra::BoundingBox3Ds &bas,
                  const algebra::BoundingBox3Ds &bbs) const {
  IMP_OBJECT_LOG;
  set_was_used(true);
  IntPairs out;
  if (merged_) {
     internal::PBBHelper::
      fill_close_pairs(internal::PBBHelper::get_particle_set(bas.begin(),
                                                   bas.end(),
                                                   internal::BBID(bas.begin()),
                                               internal::BBCenter(bas.begin()),
                                              internal::BBRadius(bas.begin())),
                       internal::PBBHelper::get_particle_set(bbs.begin(),
                                                   bbs.end(),
                                                   internal::BBID(bbs.begin()),
                                               internal::BBCenter(bbs.begin()),
                                              internal::BBRadius(bbs.begin())),
              internal::PeriodicBBClose(bas.begin(), bbs.begin(), bb_, merged_,
                                       get_distance()),
                       get_distance(), bb_, merged_,
                       internal::BBPairSink(out));
  } else {
    internal::BBHelper::
      fill_close_pairs(internal::BBHelper::get_particle_set(bas.begin(),
                                                  bas.end(),
                                                  internal::BBID(bas.begin()),
                                              internal::BBCenter(bas.begin()),
                                             internal::BBRadius(bas.begin())),
                       internal::BBHelper::get_particle_set(bbs.begin(),
                                                  bbs.end(),
                                                  internal::BBID(bbs.begin()),
                                              internal::BBCenter(bbs.begin()),
                                             internal::BBRadius(bbs.begin())),
                  internal::BBClose(bas.begin(), bbs.begin(), get_distance()),
                       get_distance(), bb_, merged_,
                       internal::BBPairSink(out));
  }
  return out;
}

IntPairs GridClosePairsFinder
::get_close_pairs(const algebra::BoundingBox3Ds &bas) const {
  IMP_OBJECT_LOG;
  set_was_used(true);
  IntPairs out;
  if (merged_) {
    internal::PBBHelper
      ::fill_close_pairs(internal::PBBHelper::get_particle_set(bas.begin(),
                                                     bas.end(),
                                               internal::BBID(bas.begin()),
                                           internal::BBCenter(bas.begin()),
                                          internal::BBRadius(bas.begin())),
                         internal::PeriodicBBClose(bas.begin(), bas.begin(),
                                         bb_, merged_, get_distance()),
                         get_distance(), bb_, merged_,
                         internal::BBPairSink(out));
  } else {
    internal::BBHelper
      ::fill_close_pairs(internal::BBHelper::get_particle_set(bas.begin(),
                                                    bas.end(),
                                                    internal::BBID(bas.begin()),
                                                internal::BBCenter(bas.begin()),
                                               internal::BBRadius(bas.begin())),
                    internal::BBClose(bas.begin(), bas.begin(), get_distance()),
                         get_distance(), bb_, merged_,
                         internal::BBPairSink(out));
  }
  return out;
}


void GridClosePairsFinder::do_show(std::ostream &out) const {
  out << "distance " << get_distance() << std::endl;
}


ParticlesTemp
GridClosePairsFinder::get_input_particles(const ParticlesTemp &ps) const {
  return ps;
}

ContainersTemp
GridClosePairsFinder::get_input_containers(const ParticlesTemp &) const {
  return ContainersTemp();
}

IMPCORE_END_NAMESPACE
