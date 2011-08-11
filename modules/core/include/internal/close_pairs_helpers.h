/**
 *  \file internal/close_pairs_helpers.h
 *  \brief utilities for rigid bodies.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_INTERNAL_CLOSE_PAIRS_HELPERS_H
#define IMPCORE_INTERNAL_CLOSE_PAIRS_HELPERS_H

#include "../core_config.h"
#include "../BoxSweepClosePairsFinder.h"
#include "../GridClosePairsFinder.h"
#include "../XYZR.h"
#include "CoreListPairContainer.h"

IMPCORE_BEGIN_INTERNAL_NAMESPACE

inline ClosePairsFinder *default_cpf(unsigned int ) {
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
inline void filter_close_pairs(C *c, ParticleIndexPairs &ps) {
  for (typename C::PairFilterConstIterator it=c->pair_filters_begin();
       it != c->pair_filters_end(); ++it) {
    (*it)->filter_in_place(c->get_model(), ps);
  }
}


inline void fix_order(ParticleIndexPairs &pips) {
  for (unsigned int i=0; i< pips.size(); ++i) {
    if (pips[i][0] > pips[i][1]) {
      std::swap(pips[i][0], pips[i][1]);
    }
  }
}
/*
// Check that they are unique by checking order
struct SameParticle {
  ParticleIndexes ppt_;
  SameParticle(const ParticleIndexes &ppt): ppt_(ppt){
    std::sort(ppt_.begin(), ppt_.end());
  }
  SameParticle(){}
  bool operator()(const ParticleIndexPair& pp) {
    if (ppt_.empty()) {
      return pp[0] == pp[1];
    } else {
      if (binary_search(ppt_.begin(), ppt_.end(), pp[0])
          && binary_search(ppt_.begin(), ppt_.end(), pp[1])) {
        return !(pp[0] < pp[1]);
      } else {
        return false;
      }
    }
  }
  };*/

// Check that they are unique by checking order
struct SameParticle {
  SameParticle(){
  }
  bool operator()(const ParticleIndexPair& pp) {
    return pp[0] == pp[1];
  }
};

/*

inline void filter_same(ParticleIndexPairs &c,
                        const ParticleIndexes &moved) {
  c.erase(std::remove_if(c.begin(), c.end(),
                         SameParticle(moved)),
          c.end());
          }*/

inline void filter_same(ParticleIndexPairs &c) {
  c.erase(std::remove_if(c.begin(), c.end(),
                         SameParticle()),
          c.end());
}


inline bool get_are_close(Model *m, ParticleIndex a, ParticleIndex b,
                          double distance) {
  XYZ da(m, a);
  XYZ db(m, b);
  Float ra= XYZR(m, a).get_radius();
  Float rb= XYZR(m, b).get_radius();
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
  Model *m_;
  FarParticle( Model *m, double d): d_(d), m_(m){}
  bool operator()(const ParticleIndexPair& pp) const {
    return !get_are_close(m_, pp[0], pp[1], d_);
  }
};

inline void filter_far(Model *m, ParticleIndexPairs &c, double d) {
  c.erase(std::remove_if(c.begin(), c.end(),
                         FarParticle(m, d)),
          c.end());
}


struct InList {
  static IntKey key_;
  const ParticlesTemp &ps_;
  InList( const ParticlesTemp &ps): ps_(ps){
  }
  static InList create( ParticlesTemp &ps) {
    /*for (unsigned int i=0; i< ps.size(); ++i) {
      ps[i]->add_attribute(key_, 1);
      }*/
    std::sort(ps.begin(), ps.end());
    return InList(ps);
  }
  static void destroy(InList &) {
    /*for (unsigned int i=0; i< il.ps_.size(); ++i) {
      il.ps_[i]->remove_attribute(key_);
      }*/
  }
  bool operator()(const ParticlePair &pp) const {
    if (std::binary_search(ps_.begin(), ps_.end(), pp[0])) return true;
    else if (std::binary_search(ps_.begin(), ps_.end(), pp[1])) return true;
    return false;
    //return pp[0]->has_attribute(key_) || pp[1]->has_attribute(key_);
  }
};


IMPCORE_END_INTERNAL_NAMESPACE

#endif  /* IMPCORE_INTERNAL_CLOSE_PAIRS_HELPERS_H */
