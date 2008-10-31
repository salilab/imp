/**
 *  \file ClosePairsFinder.cpp
 *  \brief Algorithm base class to find close pairs.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/core/ClosePairsFinder.h"
#include "IMP/core/ListParticleContainer.h"
#include <algorithm>

IMPCORE_BEGIN_NAMESPACE

ClosePairsFinder::ClosePairsFinder(){}

ClosePairsFinder::~ClosePairsFinder(){}

namespace {
  struct TestAndAdd{
    FilteredListParticlePairContainer *out_;
    ParticleContainer *a_, *b_;
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

/*
void ClosePairsFinder
::add_close_pairs(ParticleContainer *ca,
                  ParticleContainer *cb,
                  Float distance,
                  FloatKey radius_key,
                  FilteredListParticlePairContainer *out) {

  Pointer<FilteredListParticlePairContainer>
    lppc(new FilteredListParticlePairContainer());
  {
    Pointer<ListParticleContainer> lpc(new ListParticleContainer());
    Particles ps(ca->particles_begin(), ca->particles_end());
    ps.insert(ps.end(), cb->particles_begin(), cb->particles_end());
    lpc->add_particles(ps);
    add_close_pairs(lpc, distance, radius_key, lppc);
  }
  std::for_each(lppc->particle_pairs_begin(),
                lppc->particle_pairs_end(),
                TestAndAdd(ca, cb, out));
}

void ClosePairsFinder
::add_close_pairs(ParticleContainer *c,
                  Float distance,
                  FloatKey radius_key,
                  FilteredListParticlePairContainer *out) {
  add_close_pairs(c, c, distance, radius_key, out);
}
*/

void ClosePairsFinder::show(std::ostream &out) const {
  out << "ClosePairsFinder" << std::endl;
}

IMPCORE_END_NAMESPACE
