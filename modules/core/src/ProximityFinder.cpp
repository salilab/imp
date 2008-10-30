/**
 *  \file ProximityFinder.cpp
 *  \brief Algorithm base class to find proximal pairs.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/core/ProximityFinder.h"
#include "IMP/core/ListParticleContainer.h"
#include <algorithm>

IMPCORE_BEGIN_NAMESPACE

ProximityFinder::ProximityFinder(){}

ProximityFinder::~ProximityFinder(){}

namespace {
  struct TestAndAdd{
    Pointer<FilteredListParticlePairContainer> out_;
    Pointer<ParticleContainer> a_, b_;
    TestAndAdd(ParticleContainer *a, ParticleContainer *b,
               FilteredListParticlePairContainer *out): out_(out),
                                                        a_(a), b_(b){}
    void operator()(ParticlePair p) {
      if (a_->get_contains_particle(p.first)
          && b_->get_contains_particle(p.second)) {
        out_->add_particle_pair(p);
      }
    }
  };
}

void ProximityFinder
::add_proximal_pairs(ParticleContainer *ca,
                     ParticleContainer *cb,
                     FilteredListParticlePairContainer *out) {

  Pointer<FilteredListParticlePairContainer>
    lppc(new FilteredListParticlePairContainer());
  {
    Pointer<ListParticleContainer> lpc(new ListParticleContainer());
    Particles ps(ca->particles_begin(), ca->particles_end());
    ps.insert(ps.end(), cb->particles_begin(), cb->particles_end());
    lpc->add_particles(ps);
    add_proximal_pairs(lpc, lppc);
  }
  std::for_each(lppc->particle_pairs_begin(),
                lppc->particle_pairs_end(),
                TestAndAdd(ca, cb, out));
}

void ProximityFinder
::add_proximal_pairs(ParticleContainer *c,
                     FilteredListParticlePairContainer *out) {
  add_proximal_pairs(c, c, out);
}


IMPCORE_END_NAMESPACE
