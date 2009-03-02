/**
 *  \file container_helpers.h
 *  \brief Internal helpers for container classes.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_INTERNAL_CONTAINER_HELPERS_H
#define IMP_INTERNAL_CONTAINER_HELPERS_H

#include "../base_types.h"
#include "../Particle.h"
#include "../SingletonScore.h"
#include "../PairScore.h"

#include <boost/tuple/tuple.hpp>

IMP_BEGIN_INTERNAL_NAMESPACE

template <class P>
struct ContainerTraits {
  static const bool is_singleton=false;
};

template <>
struct ContainerTraits<Particle> {
  static const bool is_singleton=true;
  static Particles create_set(Particle*p) {return Particles();}
  static Float evaluate(SingletonScore *ss,
                        Particle *p,
                        DerivativeAccumulator *ac) {
    return ss->evaluate(p, ac);
  }
  template <class SM>
  static void apply(SM *ss,
                    Particle *p) {
    ss->apply(p);
  }
  template <class SM>
  static void apply(SM *ss,
                    Particle *p,
                    DerivativeAccumulator *da) {
    ss->apply(p, da);
  }
};

template <>
struct ContainerTraits<ParticlePair> {
  static const bool is_singleton=false;
  static Particles create_set(ParticlePair p) {
    Particles ret;
    ret.push_back(p.first);
    ret.push_back(p.second);
    return ret;
  }
  static Float evaluate(PairScore *ss,
                        ParticlePair p,
                        DerivativeAccumulator *ac) {
    return ss->evaluate(p.first, p.second, ac);
  }
  template <class PM>
  static void apply(PM *ss,
                    const ParticlePair &p) {
    ss->apply(p.first, p.second);
  }
  template <class PM>
  static void apply(PM *ss,
                    const ParticlePair &p,
                    DerivativeAccumulator *da) {
    ss->apply(p.first, p.second, da);
  }
};

/*template <>
struct ContainerTraits<ParticleTriplet> {
  static const bool is_singleton=false;
  static Particles create_set(ParticleTriplet p) {
    Particles ret;
    ret.push_back(p.first);
    ret.push_back(p.second);
    ret.push_back(p.third);
    return ret;
  }
  static Float evaluate(TripletScore *ss,
                        ParticleTriplet p,
                        DerivativeAccumulator *ac) {
    return ss->evaluate(p.first, p.second, p.third, ac);
  }
  static void apply(TripletModifier *ss,
                     const ParticleTriplet &p) {
    ss->apply(p.first, p.second, p.third);
  }

};*/


inline const Particle& streamable(Particle *p) {
  return *p;
}

inline const ParticlePair& streamable(const ParticlePair &p) {
  return p;
}

IMP_END_INTERNAL_NAMESPACE

#endif /* IMP_INTERNAL_CONTAINER_HELPERS_H */
