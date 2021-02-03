/**
 *  \file container_helpers.h
 *  \brief Internal helpers for container classes.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INTERNAL_CONTAINER_HELPERS_H
#define IMPKERNEL_INTERNAL_CONTAINER_HELPERS_H

#include "../base_types.h"
#include "../Particle.h"
#include "../particle_index.h"
#include "../Model.h"
#include "../ModelObject.h"
#include "utility.h"
#include <boost/version.hpp>
#if BOOST_VERSION >= 107300
#include <boost/bind/bind.hpp>
#else
#include <boost/bind.hpp>
#endif
#include <boost/bind/placeholders.hpp>
#include <boost/tuple/tuple.hpp>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

inline bool is_valid(Particle *p) { return p; }
template <unsigned int D>
inline bool is_valid(
    const Array<D, WeakPointer<Particle>, Particle *> &p) {
  for (unsigned int i = 0; i < D; ++i) {
    if (!p[i]) return false;
  }
  return true;
}

template <unsigned int D>
inline ParticlesTemp flatten(const Vector<
    Array<D, WeakPointer<Particle>, Particle *> > &in) {
  ParticlesTemp ret(in.size() * D);
  for (unsigned int i = 0; i < in.size(); ++i) {
    for (unsigned int j = 0; j < D; ++j) {
      ret[i * D + j] = in[i][j];
    }
  }
  return ret;
}

template <unsigned int D>
inline ParticleIndexes flatten(
    const Vector<Array<D, ParticleIndex> > &in) {
  ParticleIndexes ret(in.size() * D);
  for (unsigned int i = 0; i < in.size(); ++i) {
    for (unsigned int j = 0; j < D; ++j) {
      ret[i * D + j] = in[i][j];
    }
  }
  return ret;
}

inline ParticlesTemp flatten(const ParticlesTemp &in) { return in; }
inline ParticleIndexes flatten(const ParticleIndexes &in) { return in; }

inline ParticlesTemp flatten(const Particles &in) {
  return get_as<ParticlesTemp>(in);
}

inline ParticlesTemp flatten(Particle *p) { return ParticlesTemp(1, p); }
inline ParticleIndexes flatten(ParticleIndex p) {
  return ParticleIndexes(1, p);
}

template <unsigned int D>
inline ParticlesTemp flatten(
    const Array<D, WeakPointer<Particle>, Particle *> pt) {
  return ParticlesTemp(pt.begin(), pt.end());
}
template <unsigned int D>
inline ParticleIndexes flatten(const Array<D, ParticleIndex> pt) {
  return ParticleIndexes(pt.begin(), pt.end());
}

inline std::string streamable(Particle *p) { return p->get_name(); }

template <unsigned int D>
inline std::string streamable(
    const Array<D, WeakPointer<Particle>, Particle *> &p) {
  std::ostringstream oss;
  oss << p;
  return oss.str();
}

inline Particle *get_particle(Model *m, ParticleIndex pi) {
  IMP_USAGE_CHECK(m, "nullptr passed for the Model.");
  return m->get_particle(pi);
}
template <unsigned int D>
Array<D, WeakPointer<Particle>, Particle *> get_particle(
    Model *m, const Array<D, ParticleIndex> &in) {
  Array<D, WeakPointer<Particle>, Particle *> ret;
  for (unsigned int i = 0; i < D; ++i) {
    ret[i] = get_particle(m, in[i]);
    IMP_CHECK_OBJECT(ret[i]);
  }
  return ret;
}

inline ParticlesTemp get_particle(Model *m, const ParticleIndexes &ps) {
  ParticlesTemp ret(ps.size());
  for (unsigned int i = 0; i < ps.size(); ++i) {
    ret[i] = get_particle(m, ps[i]);
  }
  return ret;
}

template <unsigned int D>
inline Vector<Array<D, WeakPointer<Particle>, Particle *> >
get_particle(Model *m,
             const Vector<Array<D, ParticleIndex> > &ps) {
  Vector<Array<D, WeakPointer<Particle>, Particle *> > ret(
      ps.size());
  for (unsigned int i = 0; i < ps.size(); ++i) {
    ret[i] = get_particle(m, ps[i]);
  }
  return ret;
}

inline ParticleIndex get_index(Particle *p) {
  IMP_CHECK_OBJECT(p);
  return p->get_index();
}
template <unsigned int D>
Array<D, ParticleIndex> get_index(
    const Array<D, WeakPointer<Particle>, Particle *> &in) {
  Array<D, ParticleIndex> ret;
  for (unsigned int i = 0; i < D; ++i) {
    ret[i] = get_index(in[i]);
  }
  return ret;
}

inline ParticleIndexes get_index(const ParticlesTemp &p) {
  ParticleIndexes ret(p.size(), get_invalid_index<ParticleIndexTag>());
  for (unsigned int i = 0; i < ret.size(); ++i) {
    ret[i] = get_index(p[i]);
  }
  return ret;
}
template <unsigned int D>
Vector<Array<D, ParticleIndex> > get_index(const Vector<
    Array<D, WeakPointer<Particle>, Particle *> > &in) {
  Vector<Array<D, ParticleIndex> > ret(in.size());
  for (unsigned int i = 0; i < ret.size(); ++i) {
    Array<D, ParticleIndex> c;
    for (unsigned int j = 0; j < D; ++j) {
      c[j] = get_index(in[i][j]);
    }
    ret[i] = c;
  }
  return ret;
}

inline Model *get_model(Particle *p) { return p->get_model(); }
inline Model *get_model(ModelObject *p) { return p->get_model(); }
inline Model *get_model(ScoreState *p) { return p->get_model(); }

inline Model *get_model(const ParticlesTemp &p) {
  IMP_USAGE_CHECK(p.size() > 0, "Empty particles list");
  return get_model(p[0]);
}

template <unsigned int D>
inline Model *get_model(
    const Array<D, WeakPointer<Particle>, Particle *> &p) {
  return p[0]->get_model();
}

template <class C>
inline Model *get_model(const Vector<C> &p) {
  IMP_USAGE_CHECK(p.size() > 0, "There must be some particle tuples passed");
  return get_model(p[0]);
}

inline std::string get_name(Particle *p) { return p->get_name(); }

template <unsigned int D>
inline std::string get_name(
    const Array<D, WeakPointer<Particle>, Particle *> &p) {
  return p.get_name();
}

inline ParticleIndex get_canonical(ParticleIndex p) { return p; }

template <unsigned int D>
inline Array<D, ParticleIndex> get_canonical(
    Array<D, ParticleIndex> p) {
  std::sort(p.begin(), p.end());
  return p;
}

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_INTERNAL_CONTAINER_HELPERS_H */
