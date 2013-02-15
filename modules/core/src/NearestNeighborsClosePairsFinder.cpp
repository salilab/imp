/**
 *  \file NearestNeighborsClosePairsFinder.cpp
 *  \brief Test all pairs.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/base_types.h>
#include "IMP/core/NearestNeighborsClosePairsFinder.h"
#include "IMP/core/XYZR.h"
#include <IMP/algebra/vector_search.h>
#include <IMP/macros.h>

IMPCORE_BEGIN_NAMESPACE


namespace {

  template <class It>
    double max_radius(It b, It e) {
    double m=0;
    for (It c= b; c!= e; ++c) {
      XYZR d(*c);
      m= std::max(d.get_radius(), m);
    }
    return m;
  }


}


NearestNeighborsClosePairsFinder::NearestNeighborsClosePairsFinder():
  ClosePairsFinder("NearestNeighborsCPF") {}

ParticlePairsTemp NearestNeighborsClosePairsFinder
::get_close_pairs(
#if !defined(__clang__)
IMP_RESTRICT const ParticlesTemp &pa,
IMP_RESTRICT const ParticlesTemp &pb
#else
const ParticlesTemp &pa,
const ParticlesTemp &pb
#endif
) const {
  IMP_NEW(algebra::NearestNeighborD<3>, nn, (pa.begin(),
                                             pa.end(), 0));
  double rm= max_radius(pa.begin(), pa.end());
  ParticlePairsTemp ret;
  for (unsigned int i=0; i< pb.size(); ++i) {
      XYZR d(pb[i]);
      Ints cur= nn->get_in_ball(d.get_coordinates(),
                               rm + get_distance()+ d.get_radius());
      for (unsigned int j=0; j< cur.size(); ++j) {
        ret.push_back(ParticlePair(pa[cur[j]],
                                   d));
      }
  };
  return ret;
}
ParticlePairsTemp NearestNeighborsClosePairsFinder
::get_close_pairs(
#if !defined(__clang__)
IMP_RESTRICT
#endif
const ParticlesTemp &c) const {
  IMP_NEW(algebra::NearestNeighborD<3>, nn,(c.begin(), c.end(), 0));
  double rm= max_radius(c.begin(), c.end());
  ParticlePairsTemp ret;
  for (unsigned int i=0; i< c.size(); ++i) {
    XYZR d(c[i]);
    Ints cur=nn->get_in_ball(d.get_coordinates(),
                            rm + get_distance()+ d.get_radius());
    for (unsigned int j=0; j< cur.size(); ++j) {
      if (d < c[cur[j]]) {
        ret.push_back(ParticlePair(c[cur[j]],
                                   d));
      }
    }
    }
  return ret;
}

IntPairs NearestNeighborsClosePairsFinder::
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

IntPairs NearestNeighborsClosePairsFinder::
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


void NearestNeighborsClosePairsFinder::do_show(std::ostream &out) const {
  out << "distance " << get_distance() << std::endl;
}


ModelObjectsTemp
NearestNeighborsClosePairsFinder::do_get_inputs(Model *m,
                                    const ParticleIndexes &pis) const {
  return get_particles(m, pis);
}

IMPCORE_END_NAMESPACE
