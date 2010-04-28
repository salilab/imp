/**
 *  \file NearestNeighborsClosePairsFinder.cpp
 *  \brief Test all pairs.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
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
::get_close_pairs(IMP_RESTRICT SingletonContainer *ca,
                  IMP_RESTRICT SingletonContainer *cb) const {
  algebra::NearestNeighborD<3> nn(ca->particles_begin(),
                                  ca->particles_end(), 0);
  double rm= max_radius(ca->particles_begin(), ca->particles_end());
  ParticlePairsTemp ret;
  IMP_FOREACH_SINGLETON(cb, {
      XYZR d(_1);
      Ints cur= nn.get_in_ball(d.get_coordinates(),
                               rm + get_distance()+ d.get_radius());
      for (unsigned int j=0; j< cur.size(); ++j) {
        ret.push_back(ParticlePair(ca->get_particle(cur[j]),
                                   d));
      }
    });
  return ret;
}
ParticlePairsTemp NearestNeighborsClosePairsFinder
::get_close_pairs(IMP_RESTRICT SingletonContainer *c) const {
  algebra::NearestNeighborD<3> nn(c->particles_begin(), c->particles_end(), 0);
  double rm= max_radius(c->particles_begin(), c->particles_end());
  ParticlePairsTemp ret;
  IMP_FOREACH_SINGLETON(c, {
    XYZR d(_1);
    Ints cur=nn.get_in_ball(d.get_coordinates(),
                            rm + get_distance()+ d.get_radius());
    for (unsigned int j=1; j< cur.size(); ++j) {
      ret.push_back(ParticlePair(c->get_particle(cur[j]),
                                 d));
    }
    });
  return ret;
}

void NearestNeighborsClosePairsFinder::do_show(std::ostream &out) const {
  out << "distance " << get_distance() << std::endl;
}


ParticlesTemp
NearestNeighborsClosePairsFinder
::get_input_particles(SingletonContainer *sc) const {
  ParticlesTemp ret= sc->get_particles();
  return ret;
}

ParticlesTemp
NearestNeighborsClosePairsFinder::get_input_particles(SingletonContainer *a,
                                             SingletonContainer *b) const {
  ParticlesTemp ret0= a->get_particles();
  ParticlesTemp ret1= b->get_particles();
  ret0.insert(ret0.end(), ret1.begin(), ret1.end());
  return ret0;
}

ContainersTemp
NearestNeighborsClosePairsFinder
::get_input_containers(SingletonContainer *sc) const {
  ContainersTemp ret(1,sc);
  return ret;
}

ContainersTemp
NearestNeighborsClosePairsFinder::get_input_containers(SingletonContainer *a,
                                              SingletonContainer *b) const {
  ContainersTemp ret(2);
  ret[0]= a;
  ret[1]= b;
  return ret;
}

IMPCORE_END_NAMESPACE
