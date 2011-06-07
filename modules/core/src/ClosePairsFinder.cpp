/**
 *  \file ClosePairsFinder.cpp
 *  \brief Algorithm base class to find close pairs.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/core/ClosePairsFinder.h"
#include "IMP/core/internal/MovedSingletonContainer.h"
#include <limits>

IMPCORE_BEGIN_NAMESPACE

ClosePairsFinder::ClosePairsFinder(std::string name): Object(name),
  distance_(std::numeric_limits<double>::quiet_NaN()){
  set_was_used(true);
}

ClosePairsFinder::~ClosePairsFinder(){}

internal::MovedSingletonContainer*
ClosePairsFinder::get_moved_singleton_container(SingletonContainer *in,
                                                 Model *m,
                                                 double threshold) const {
  return new internal::MovedSingletonContainerImpl<algebra::SphereD<3>,
    internal::SaveXYZRValues,
    internal::SaveMovedValues<internal::SaveXYZRValues>,
    internal::ListXYZRMovedParticles>(m, in, threshold);
}




ParticlePairsTemp ClosePairsFinder
::get_close_pairs(const ParticlesTemp &pta,
                  const ParticlesTemp &ptb) const {
  set_was_used(true);
  IMP_OBJECT_LOG;
  IMP_LOG(TERSE, "Quadratic add_close_pairs called with "
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

ParticlePairsTemp ClosePairsFinder
::get_close_pairs(const ParticlesTemp &pt) const {
  set_was_used(true);
  IMP_OBJECT_LOG;
  IMP_LOG(TERSE, "Adding close pairs from "
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


IntPairs ClosePairsFinder::
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

IntPairs ClosePairsFinder::
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


ParticlesTemp
ClosePairsFinder::get_input_particles(const ParticlesTemp &ps) const {
  return ps;
}



ContainersTemp
ClosePairsFinder::get_input_containers(const ParticlesTemp &) const {
  return ContainersTemp();
}


IMPCORE_END_NAMESPACE
