/**
 *  \file container_helpers.h
 *  \brief Internal helpers for container classes.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INTERNAL_CONTAINER_HELPERS_H
#define IMPKERNEL_INTERNAL_CONTAINER_HELPERS_H

#include "../base_types.h"
#include "../declare_Particle.h"
#include "../ParticleTuple.h"
#include "../particle_index.h"
#include "../declare_Model.h"
#include "../ModelObject.h"
#include "utility.h"
#include <boost/bind.hpp>
#include <boost/bind/placeholders.hpp>
#include <boost/tuple/tuple.hpp>


IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

inline bool is_valid(Particle *p) {
  return p;
}
template <unsigned int D>
inline bool is_valid(const base::Array<D, base::WeakPointer<Particle>,
                     Particle*> &p) {
  for (unsigned int i=0; i< D; ++i) {
    if (!p[i]) return false;
  }
  return true;
}

template <unsigned int D>
inline ParticlesTemp
flatten(const base::Vector<base::Array<D, base::WeakPointer<Particle>,
                                       Particle*> > &in) {
  ParticlesTemp ret(in.size()*D);
  for (unsigned int i=0; i< in.size(); ++i) {
    for (unsigned int j=0; j< D; ++j) {
      ret[i*D+j]= in[i][j];
    }
  }
  return ret;
}

template <unsigned int D>
inline ParticleIndexes
flatten(const base::Vector<base::Array<D, ParticleIndex> > &in) {
  ParticleIndexes ret(in.size()*D);
  for (unsigned int i=0; i< in.size(); ++i) {
    for (unsigned int j=0; j< D; ++j) {
      ret[i*D+j]= in[i][j];
    }
  }
  return ret;
}

inline ParticlesTemp flatten(const ParticlesTemp &in) {
  return in;
}
inline ParticleIndexes flatten(const ParticleIndexes &in) {
  return in;
}

inline ParticlesTemp flatten(const Particles &in) {
  return get_as<ParticlesTemp>(in);
}

inline ParticlesTemp flatten(Particle *p) {return ParticlesTemp(1,p);}
inline ParticleIndexes flatten(ParticleIndex p) {return ParticleIndexes(1,p);}

template <unsigned int D>
inline ParticlesTemp flatten(const base::Array<D, base::WeakPointer<Particle>,
                                               Particle*> pt) {
  return ParticlesTemp(pt.begin(), pt.end());
}
template <unsigned int D>
inline ParticleIndexes flatten(const base::Array<D, ParticleIndex> pt) {
  return ParticleIndexes(pt.begin(), pt.end());
}


inline std::string streamable(Particle *p) {
  return p->get_name();
}

template <unsigned int D>
inline std::string streamable(const base::Array<D, base::WeakPointer<Particle>,
                                                Particle*> &p) {
  std::ostringstream oss;
  oss << p;
  return oss.str();
}



template <unsigned int D, class Score>
IMP_DEPRECATED_WARN inline ContainersTemp
get_input_containers(Score *s,
                     const base::Array<D, base::WeakPointer<Particle>,
                                       Particle*>& p) {
  ContainersTemp ret;
  for (unsigned int i=0; i< D; ++i) {
    ContainersTemp c= s->get_input_containers(p[i]);
    ret.insert(ret.end(), c.begin(), c.end());
  }
  return ret;
}

template <unsigned int D, class Score>
IMP_DEPRECATED_WARN inline ContainersTemp
get_output_containers(Score *s,
                      const base::Array<D, base::WeakPointer<Particle>,
                                        Particle*>& p) {
  ContainersTemp ret;
  for (unsigned int i=0; i< D; ++i) {
    ContainersTemp c= s->get_output_containers(p[i]);
    ret.insert(ret.end(), c.begin(), c.end());
  }
  return ret;
}

template <class Score>
IMP_DEPRECATED_WARN
inline ContainersTemp get_input_containers(Score *s,
                                     Particle * const p) {
  return s->get_input_containers(p);
}

template <class Score>
IMP_DEPRECATED_WARN
inline ContainersTemp get_output_containers(Score *s,
                                     Particle* const p) {
  return s->get_output_containers(p);
}


template <class Score, class C>
IMP_DEPRECATED_WARN
inline ParticlesTemp get_output_particles(Score *s,
                                   const C& p) {
  ParticlesTemp ret;
  for (unsigned int i=0; i< p.size(); ++i) {
    ParticlesTemp c= s->get_output_particles(p[i]);
    ret.insert(ret.end(), c.begin(), c.end());
  }
  return ret;
}
template <class Score, class C>
IMP_DEPRECATED_WARN
inline ParticlesTemp get_input_particles(Score *s,
                                  const C& p) {
  ParticlesTemp ret;
  for (unsigned int i=0; i< p.size(); ++i) {
    ParticlesTemp c= s->get_input_particles(p[i]);
    ret.insert(ret.end(), c.begin(), c.end());
  }
  return ret;
}
template <class Score, class C>
IMP_DEPRECATED_WARN
inline ContainersTemp get_input_containers(Score *s,
                                   const C& p) {
  ContainersTemp ret;
  for (unsigned int i=0; i< p.size(); ++i) {
    ContainersTemp c= s->get_input_containers(p[i]);
    ret.insert(ret.end(), c.begin(), c.end());
  }
  return ret;
}
template <class S>
IMP_DEPRECATED_WARN
inline ParticlesTemp get_output_particles(S *s,
                                   Particle *p) {
  return s->get_output_particles(p);
}
template <class S>
IMP_DEPRECATED_WARN
inline ParticlesTemp get_input_particles(S *s,
                                     Particle *p) {
  return s->get_input_particles(p);
}

template <class S>
IMP_DEPRECATED_WARN
inline ParticlesTemp get_output_particles(S *s,
                                          base::Pointer<Particle> p) {
  return s->get_output_particles(p);
}
template <class S>
IMP_DEPRECATED_WARN
inline ParticlesTemp get_input_particles(S *s,
                                         base::Pointer<Particle> p) {
  return s->get_input_particles(p);
}
template <class S>
IMP_DEPRECATED_WARN
inline ContainersTemp get_input_containers(S *s,
                                           base::Pointer<Particle> p) {
  return s->get_input_containers(p);
}


inline
Particle* get_particle(Model *m, ParticleIndex pi) {
  IMP_USAGE_CHECK(m, "nullptr passed for the Model.");
  return m->get_particle(pi);
}
template <unsigned int D>
base::Array<D, base::WeakPointer<Particle>, Particle*>
get_particle(Model *m, const base::Array<D, ParticleIndex> &in) {
  base::Array<D, base::WeakPointer<Particle>, Particle*> ret;
  for (unsigned int i=0; i< D; ++i) {
    ret[i]= get_particle(m, in[i]);
    IMP_CHECK_OBJECT(ret[i]);
  }
  return ret;
}

inline ParticlesTemp
get_particle(Model *m, const ParticleIndexes &ps) {
  ParticlesTemp ret(ps.size());
  for (unsigned int i=0; i< ps.size(); ++i) {
    ret[i]= get_particle(m, ps[i]);
  }
  return ret;
}

template <unsigned int D>
inline
base::Vector<base::Array<D, base::WeakPointer<Particle>, Particle*> >
get_particle(Model *m,
        const base::Vector<base::Array<D, ParticleIndex> > &ps) {
  base::Vector<base::Array<D, base::WeakPointer<Particle>, Particle*> >
    ret(ps.size());
  for (unsigned int i=0; i< ps.size(); ++i) {
    ret[i]= get_particle(m, ps[i]);
  }
  return ret;
}

inline
ParticleIndex get_index(Particle*p) {
  IMP_CHECK_OBJECT(p);
  return p->get_index();
}
template <unsigned int D>
base::Array<D, ParticleIndex>
get_index(const base::Array<D, base::WeakPointer<Particle>, Particle*> &in) {
  base::Array<D, ParticleIndex> ret;
  for (unsigned int i=0; i< D; ++i) {
    ret[i]= get_index(in[i]);
  }
  return ret;
}

inline
ParticleIndexes get_index(const ParticlesTemp& p) {
  ParticleIndexes ret(p.size(), base::get_invalid_index<ParticleIndexTag>());
  for (unsigned int i=0; i< ret.size(); ++i) {
    ret[i]= get_index(p[i]);
  }
  return ret;
}
template <unsigned int D>
base::Vector<base::Array<D, ParticleIndex> >
get_index(const base::Vector<base::Array<D,
                                         base::WeakPointer<Particle>,
                                         Particle*> > &in) {
  base::Vector<base::Array<D, ParticleIndex> > ret(in.size());
  for (unsigned int i=0; i< ret.size(); ++i) {
    base::Array<D, ParticleIndex> c;
    for (unsigned int j=0; j< D; ++j) {
      c[j]= get_index(in[i][j]);
    }
    ret[i]=c;
  }
  return ret;
}


inline Model *get_model(Particle*p) {
  return p->get_model();
}
inline Model *get_model(ModelObject*p) {
  return p->get_model();
}
inline Model *get_model(ScoreState*p) {
  return p->get_model();
}


inline Model *get_model(const ParticlesTemp&p) {
  IMP_USAGE_CHECK(p.size() >0, "Empty particles list");
  return get_model(p[0]);
}

template <unsigned int D>
inline Model *get_model(const base::Array<D, base::WeakPointer<Particle>,
                                          Particle*>& p) {
  return p[0]->get_model();
}

template <class C>
inline Model *get_model(const base::Vector<C>& p) {
  IMP_USAGE_CHECK(p.size() > 0,
    "There must be some particle tuples passed");
return get_model(p[0]);
}

inline std::string get_name(Particle*p) {
  return p->get_name();
}

template <unsigned int D>
inline std::string get_name(const base::Array<D,
                              base::WeakPointer<Particle>, Particle*>& p) {
  return p.get_name();
}

inline
ParticleIndex get_canonical(ParticleIndex p) {return p;}

template <unsigned int D>
inline
base::Array<D,ParticleIndex> get_canonical(base::Array<D,ParticleIndex> p) {
  std::sort(p.begin(), p.end());
  return p;
}

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_INTERNAL_CONTAINER_HELPERS_H */
