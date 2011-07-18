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
                        internal::ParticlePairSink(out));
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
                       internal::ParticlePairSink(out));
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
