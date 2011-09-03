/**
 *  \file container_helpers.h
 *  \brief Internal helpers for container classes.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_INTERNAL_CONTAINER_HELPERS_H
#define IMP_INTERNAL_CONTAINER_HELPERS_H

#include "../base_types.h"
#include "../Particle.h"
#include "../ScoreState.h"
#include "utility.h"
#include <boost/bind.hpp>
#include <boost/bind/placeholders.hpp>
#include <boost/tuple/tuple.hpp>


IMP_BEGIN_INTERNAL_NAMESPACE
template <class Score, class Enabled=void>
struct SimpleRestraintParentTraits{};


inline bool is_valid(Particle *p) {
  return p;
}
template <unsigned int D>
inline bool is_valid(const ParticleTuple<D> &p) {
  for (unsigned int i=0; i< D; ++i) {
    if (!p[i]) return false;
  }
  return true;
}


inline bool is_inactive(const Particle *p) {
  return !p->get_is_active();
}
template <unsigned int D>
inline bool is_inactive(const ParticleTuple<D> &p) {
  for (unsigned int i=0; i< D; ++i) {
    if (!p[i]->get_is_active()) return true;
  }
  return false;
}

struct IsInactive {
  template <class T>
  bool operator()(const T& t) {
    return is_inactive(t);
  }
};

template <class VT>
inline ParticlesTemp flatten(const VT &in) {
  typedef typename VT::value_type T;
  ParticlesTemp ret(in.size()*T::get_dimension());
  for (unsigned int i=0; i< in.size(); ++i) {
    for (unsigned int j=0; j< T::get_dimension(); ++j) {
      ret[i*T::get_dimension()+j]= in[i][j];
    }
  }
  return ret;
}

inline const ParticlesTemp& flatten(const ParticlesTemp &in) {
  return in;
}

inline ParticlesTemp flatten(const Particles &in) {
  return get_as<ParticlesTemp>(in);
}



inline std::string streamable(Particle *p) {
  return p->get_name();
}

template <unsigned int D>
inline std::string streamable(const ParticleTuple<D> &p) {
  std::ostringstream oss;
  oss << p;
  return oss.str();
}



template <unsigned int D, class Score>
inline ContainersTemp get_input_containers(Score *s,
                                     const ParticleTuple<D>& p) {
  ContainersTemp ret;
  for (unsigned int i=0; i< D; ++i) {
    ContainersTemp c= s->get_input_containers(p[i]);
    ret.insert(ret.end(), c.begin(), c.end());
  }
  return ret;
}

template <unsigned int D, class Score>
inline ContainersTemp get_output_containers(Score *s,
                                     const ParticleTuple<D>& p) {
  ContainersTemp ret;
  for (unsigned int i=0; i< D; ++i) {
    ContainersTemp c= s->get_output_containers(p[i]);
    ret.insert(ret.end(), c.begin(), c.end());
  }
  return ret;
}

template <class Score>
inline ContainersTemp get_input_containers(Score *s,
                                     Particle * const p) {
  return s->get_input_containers(p);
}

template <class Score>
inline ContainersTemp get_output_containers(Score *s,
                                     Particle* const p) {
  return s->get_output_containers(p);
}


template <class Score, class C>
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
inline ParticlesTemp get_output_particles(S *s,
                                   Particle *p) {
  return s->get_output_particles(p);
}
template <class S>
inline ParticlesTemp get_input_particles(S *s,
                                     Particle *p) {
  return s->get_input_particles(p);
}
template <class S>
inline ContainersTemp get_input_containers(S *s,
                                   Particle *p) {
  return s->get_input_containers(p);
}
template <class S>
inline ParticlesTemp get_output_particles(S *s,
                                   Pointer<Particle> p) {
  return s->get_output_particles(p);
}
template <class S>
inline ParticlesTemp get_input_particles(S *s,
                                  Pointer<Particle> p) {
  return s->get_input_particles(p);
}
template <class S>
inline ContainersTemp get_input_containers(S *s,
                                   Pointer<Particle> p) {
  return s->get_input_containers(p);
}


inline
Particle* get_particle(Model *m, ParticleIndex pi) {
  return m->get_particle(pi);
}
template <unsigned int D>
ParticleTuple<D> get_particle(Model *m, const ParticleIndexTuple<D> &in) {
  ParticleTuple<D> ret;
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
compatibility::checked_vector<ParticleTuple<D> > get_particle(Model *m,
        const compatibility::checked_vector<ParticleIndexTuple<D> > &ps) {
  compatibility::checked_vector<ParticleTuple<D> > ret(ps.size());
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
ParticleIndexTuple<D> get_index(const ParticleTuple<D> &in) {
  ParticleIndexTuple<D> ret;
  for (unsigned int i=0; i< D; ++i) {
    ret[i]= get_index(in[i]);
  }
  return ret;
}

inline
ParticleIndexes get_index(const ParticlesTemp& p) {
  ParticleIndexes ret(p.size());
  for (unsigned int i=0; i< ret.size(); ++i) {
    ret[i]= get_index(p[i]);
  }
  return ret;
}
template <unsigned int D>
compatibility::checked_vector<ParticleIndexTuple<D> >
get_index(const compatibility::checked_vector<ParticleTuple<D> > &in) {
  compatibility::checked_vector<ParticleIndexTuple<D> > ret(in.size());
  for (unsigned int i=0; i< ret.size(); ++i) {
    ParticleIndexTuple<D> c;
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

inline Model *get_model(const ParticlesTemp&p) {
  IMP_USAGE_CHECK(p.size() >0, "Empty particles list");
  return get_model(p[0]);
}

template <unsigned int D>
inline Model *get_model(const ParticleTuple<D>& p) {
  return p[0]->get_model();
}

inline std::string get_name(Particle*p) {
  return p->get_name();
}

template <unsigned int D>
inline std::string get_name(const ParticleTuple<D>& p) {
  return p.get_name();
}


template <class Filter>
class GetContains {
  const Filter* back_;
public:
  GetContains(const Filter *n): back_(n){}
  template <class T>
  bool operator()(const T &p) const {
    return back_->get_contains(get_model(p),
                               IMP::internal::get_index(p));
  }
};

template <class Filter>
class GetContainsIndex {
  const Filter* back_;
  Model *m_;
public:
  GetContainsIndex(const Filter *n,
              Model *m): back_(n), m_(m){}
  template <class T>
  bool operator()(const T &p) const {
    return back_->get_contains(m_, p);
  }
};



#define IMP_ACTIVE_CONTAINER_DECL(Name)                         \
  private:                                                      \
  unsigned int eval_update_;                                    \
  class Ticker: public ScoreState {                             \
    Name *back_;                                                \
  public:                                                       \
  Ticker(Name *n): ScoreState(n->get_name()+" updater"),        \
                   back_(n){                                    \
    n->eval_update_=std::numeric_limits<unsigned int>::max();   \
  }                                                             \
  IMP_SCORE_STATE(Ticker);                                      \
  };                                                            \
  friend class Ticker;                                          \
  GenericScopedScoreState<Ticker> ticker_;                      \
  unsigned int get_last_update_evaluation() const {             \
  return eval_update_;                                          \
  }                                                             \
  virtual void do_before_evaluate();                            \
  virtual void do_after_evaluate();                             \
  bool get_has_model() const { return ticker_.get_is_set();}    \
  virtual ParticlesTemp get_state_input_particles() const;      \
  virtual ContainersTemp get_state_input_containers() const;    \
  void initialize_active_container(Model *m);                   \
public:                                                         \
 void set_log_level(LogLevel l)

#define IMP_ACTIVE_CONTAINER_DEF(Name, extra_objects_log)               \
  void Name::Ticker::do_before_evaluate() {                             \
    IMP_CHECK_OBJECT(back_);                                            \
    back_->do_before_evaluate();                                        \
    back_->eval_update_= back_->get_model()->get_evaluation();          \
  }                                                                     \
  void Name::Ticker::do_after_evaluate(DerivativeAccumulator*) {        \
    IMP_CHECK_OBJECT(back_);                                            \
    back_->do_after_evaluate();                                         \
  }                                                                     \
  ContainersTemp Name::Ticker::get_input_containers() const {           \
    IMP_IF_CHECK(USAGE) {                                               \
      std::string name= back_->get_name();                              \
      {Pointer<Name> rc(back_);}                                        \
      IMP_INTERNAL_CHECK((back_)->get_is_valid(),                       \
                         "Container " << name                           \
                         << " is not ref counted properly"              \
                         << " bad things will happen.");                \
    }                                                                   \
    return back_->get_state_input_containers();                         \
  }                                                                     \
  ContainersTemp Name::Ticker::get_output_containers() const {          \
    return ContainersTemp(1, back_);                                    \
  }                                                                     \
  ParticlesTemp Name::Ticker::get_input_particles() const {             \
    return back_->get_state_input_particles();                          \
  }                                                                     \
  ParticlesTemp Name::Ticker::get_output_particles() const {            \
    return ParticlesTemp();                                             \
  }                                                                     \
  void Name::Ticker::do_show(std::ostream &out) const {                 \
    out << "back is " << *back_ << std::endl;                           \
  }                                                                     \
  void Name::initialize_active_container(Model *m) {                    \
    IMP_INTERNAL_CHECK(!get_has_model(),                                \
                       "Can only set the model once.");                 \
    IMP_LOG(TERSE, "Setting up score state for container "              \
            << get_name()<< std::endl);                                 \
    IMP_INTERNAL_CHECK(get_model(),                                     \
                       "No active updating of add/remove containers."); \
    ticker_.set(new Ticker(this), m);                                   \
  }                                                                     \
  void Name::set_log_level(LogLevel l) {                                \
    Object::set_log_level(l);                                           \
    ticker_->set_log_level(l);                                          \
    extra_objects_log;                                                  \
  }                                                                     \
  IMP_REQUIRE_SEMICOLON_NAMESPACE




IMP_END_INTERNAL_NAMESPACE

#endif /* IMP_INTERNAL_CONTAINER_HELPERS_H */
