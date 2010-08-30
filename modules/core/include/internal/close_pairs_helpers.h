/**
 *  \file internal/close_pairs_helpers.h
 *  \brief utilities for rigid bodies.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_INTERNAL_CLOSE_PAIRS_HELPERS_H
#define IMPCORE_INTERNAL_CLOSE_PAIRS_HELPERS_H

#include "../core_config.h"
#include "../BoxSweepClosePairsFinder.h"
#include "../GridClosePairsFinder.h"
#include "CoreListPairContainer.h"

IMPCORE_BEGIN_INTERNAL_NAMESPACE

inline ClosePairsFinder *default_cpf() {
  return new GridClosePairsFinder();
}

struct IsInactive {
  bool operator()(const ParticlePair &p) const {
    return !p[0]->get_is_active() || !p[1]->get_is_active();
  }
};
inline CoreListPairContainer *get_list(PairContainer *pc) {
  return dynamic_cast<CoreListPairContainer *>(pc);
}


template <class C>
void filter_close_pairs(C *c, ParticlePairsTemp &ps) {
  for (typename C::PairFilterIterator it=c->pair_filters_begin();
       it != c->pair_filters_end(); ++it) {
    (*it)->filter_in_place(ps);
  }
}



struct SameParticle {
  bool operator()(ParticlePair pp) {
    return pp[0]==pp[1];
  }
};



inline void filter_same(ParticlePairsTemp &c) {
  c.erase(std::remove_if(c.begin(), c.end(),
                         SameParticle()),
          c.end());
}


IMPCORE_END_INTERNAL_NAMESPACE

#endif  /* IMPCORE_INTERNAL_CLOSE_PAIRS_HELPERS_H */
