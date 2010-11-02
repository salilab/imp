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
#include "../XYZR.h"
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


// Check that they are unique by checking order
struct SameParticle {
  ParticlesTemp ppt_;
  SameParticle(const ParticlesTemp &ppt): ppt_(ppt){
    std::sort(ppt_.begin(), ppt_.end());
  }
  bool operator()(ParticlePair pp) {
    if (binary_search(ppt_.begin(), ppt_.end(), pp[0])
        && binary_search(ppt_.begin(), ppt_.end(), pp[1])) {
      return !(pp[0] < pp[1]);
    } else {
      return false;
    }
  }
};



inline void filter_same(ParticlePairsTemp &c,
                        const ParticlesTemp &moved) {
  c.erase(std::remove_if(c.begin(), c.end(),
                         SameParticle(moved)),
          c.end());
}


inline bool get_are_close(Particle *a, Particle *b,
                          double distance) {
  XYZ da(a);
  XYZ db(b);
  Float ra= XYZR(a).get_radius();
  Float rb= XYZR(b).get_radius();
  Float sr= ra+rb+distance;
  for (unsigned int i=0; i< 3; ++i) {
    double delta=std::abs(da.get_coordinate(i) - db.get_coordinate(i));
    if (delta >= sr) {
      return false;
    }
  }
  return get_interiors_intersect(algebra::SphereD<3>(da.get_coordinates(),
                                                     ra+distance),
                                 algebra::SphereD<3>(db.get_coordinates(), rb));
}

struct FarParticle {
  double d_;
  FarParticle(double d): d_(d){}
  bool operator()(ParticlePair pp) const {
    return !get_are_close(pp[0], pp[1], d_);
  }
};

inline void filter_far(ParticlePairsTemp &c, double d) {
  c.erase(std::remove_if(c.begin(), c.end(),
                         FarParticle(d)),
          c.end());
}


IMPCORE_END_INTERNAL_NAMESPACE

#endif  /* IMPCORE_INTERNAL_CLOSE_PAIRS_HELPERS_H */
