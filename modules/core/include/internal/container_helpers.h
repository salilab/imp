/**
 *  \file container_helpers.h
 *  \brief Internal helpers for container classes.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_INTERNAL_CONTAINER_HELPERS_H
#define IMPCORE_INTERNAL_CONTAINER_HELPERS_H

#include "../macros.h"
#include <IMP/Particle.h>
#include <IMP/SingletonScore.h>
#include <IMP/PairScore.h>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

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
};

IMPCORE_END_INTERNAL_NAMESPACE

#endif /* IMPCORE_INTERNAL_CONTAINER_HELPERS_H */
