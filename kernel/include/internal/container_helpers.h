/**
 *  \file container_helpers.h
 *  \brief Internal helpers for container classes.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_INTERNAL_CONTAINER_HELPERS_H
#define IMP_INTERNAL_CONTAINER_HELPERS_H

#include "../base_types.h"
#include "../Particle.h"
#include "../SingletonScore.h"
#include "../PairScore.h"
#include "../SingletonContainer.h"
#include "../PairContainer.h"

#include <boost/tuple/tuple.hpp>


IMP_BEGIN_INTERNAL_NAMESPACE

template <class P>
struct ContainerTraits {
  static const bool is_singleton=false;
};

template <>
struct ContainerTraits<Particle> {
  static const bool is_singleton=true;
  static ParticlesTemp create_set(Particle*p) {return ParticlesTemp();}
  static Float evaluate(const SingletonScore *ss,
                        Particle *p,
                        DerivativeAccumulator *ac) {
    return ss->evaluate(p, ac);
  }
  template <class SM>
  static void apply(const SM *ss,
                    Particle *p) {
    ss->apply(p);
  }
  template <class SM>
  static void apply(const SM *ss,
                    Particle *p,
                    DerivativeAccumulator *da) {
    ss->apply(p, *da);
  }
  static bool is_inactive(const Particle* p) {
    return !p->get_is_active();
  }
};

template <>
struct ContainerTraits<ParticlePair> {
  static const bool is_singleton=false;
  static ParticlesTemp create_set(ParticlePair p) {
    ParticlesTemp ret;
    ret.push_back(p.first);
    ret.push_back(p.second);
    return ret;
  }
  static Float evaluate(const PairScore *ss,
                        ParticlePair p,
                        DerivativeAccumulator *ac) {
    return ss->evaluate(p.first, p.second, ac);
  }
  template <class PM>
  static void apply(const PM *ss,
                    const ParticlePair &p) {
    ss->apply(p.first, p.second);
  }
  template <class PM>
  static void apply(const PM *ss,
                    const ParticlePair &p,
                    DerivativeAccumulator *da) {
    ss->apply(p.first, p.second, *da);
  }
  static bool is_inactive(const ParticlePair &p) {
    return !p[0]->get_is_active() || !p[1]->get_is_active();
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

template <class F>
ParticlesList get_interacting_particles(SingletonContainer *sc,
                                        F *f) {
  ParticlesList ret;
  for (unsigned int i=0; i< sc->get_number_of_particles(); ++i) {
    ParticlesList t= f->get_interacting_particles(sc->get_particle(i));
    if (!t.empty()) {
      ret.push_back(get_union(t));
    }
  }
  return ret;
}

template <class F>
ParticlesList get_interacting_particles(PairContainer *sc,
                                        F *f) {
  ParticlesList ret;
  for (unsigned int i=0; i< sc->get_number_of_particle_pairs(); ++i) {
    ParticlePair pp=sc->get_particle_pair(i);
    ParticlesList t= f->get_interacting_particles(pp[0], pp[1]);
    if (!t.empty()) {
      ret.push_back(get_union(t));
    }
  }
  return ret;
}


template <class F>
ParticlesList get_interacting_particles(Particle *p,
                                        F *f) {
  ParticlesList t= f->get_interacting_particles(p);
  return t;
}

template <class F>
ParticlesList get_interacting_particles(ParticlePair p,
                                        F *f) {
  ParticlesList t= f->get_interacting_particles(p[0], p[1]);
  return t;
}




template <class F>
ParticlesTemp get_used_particles(SingletonContainer *sc,
                                        F *f) {
  ParticlesTemp ret;
  for (unsigned int i=0; i< sc->get_number_of_particles(); ++i) {
    ParticlesTemp t= f->get_used_particles(sc->get_particle(i));
    ret.insert(ret.end(), t.begin(), t.end());
  }
  return ret;
}

template <class F>
ParticlesTemp get_used_particles(PairContainer *sc,
                                        F *f) {
  ParticlesTemp ret;
  for (unsigned int i=0; i< sc->get_number_of_particle_pairs(); ++i) {
    ParticlePair pp=sc->get_particle_pair(i);
    ParticlesTemp t= f->get_used_particles(pp[0], pp[1]);
    ret.insert(ret.end(), t.begin(), t.end());
  }
  return ret;
}


template <class F>
ParticlesTemp get_used_particles(Particle *p,
                                 F *f) {
  ParticlesTemp t= f->get_used_particles(p);
  return t;
}

template <class F>
ParticlesTemp get_used_particles(ParticlePair p,
                                        F *f) {
  ParticlesTemp t= f->get_used_particles(p[0], p[1]);
  return t;
}

IMP_END_INTERNAL_NAMESPACE

#endif /* IMP_INTERNAL_CONTAINER_HELPERS_H */
