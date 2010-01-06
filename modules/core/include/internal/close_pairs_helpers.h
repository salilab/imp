/**
 *  \file internal/close_pairs_helpers.h
 *  \brief utilities for rigid bodies.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_INTERNAL_CLOSE_PAIRS_HELPERS_H
#define IMPCORE_INTERNAL_CLOSE_PAIRS_HELPERS_H

#include "../config.h"
#include "../BoxSweepClosePairsFinder.h"
#include "../GridClosePairsFinder.h"
#include "../ListPairContainer.h"

IMPCORE_BEGIN_INTERNAL_NAMESPACE

inline ClosePairsFinder *default_cpf() {
#ifdef IMP_USE_CGAL
  return new BoxSweepClosePairsFinder();
#else
  return new GridClosePairsFinder();
#endif
}

struct IsInactive {
  bool operator()(const ParticlePair &p) const {
    return !p[0]->get_is_active() || !p[1]->get_is_active();
  }
};
inline ListPairContainer *get_list(PairContainer *pc) {
  return dynamic_cast<ListPairContainer *>(pc);
}
template <class C>
class Found {
  typedef typename C::PairFilterIterator It;
  It b_,e_;
public:
  Found(It b,
        It e):
    b_(b), e_(e){}
  bool operator()(ParticlePair vt) const {
    if (vt[0]==vt[1]) return true;
    for (It c=b_; c != e_; ++c) {
      if ((*c)->get_contains_particle_pair(vt)) return true;
    }
    return false;
  }
};

template <class C>
void filter_close_pairs(C *c, ParticlePairsTemp &ps) {
  ps.erase(std::remove_if(ps.begin(), ps.end(),
                          Found<C>(c->pair_filters_begin(),
                                   c->pair_filters_end())),
           ps.end());
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
