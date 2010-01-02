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
#include "utility.h"

#include <boost/tuple/tuple.hpp>


IMP_BEGIN_INTERNAL_NAMESPACE


inline bool is_valid(Particle *p) {
  return p;
}
template <unsigned int D>
bool is_valid(const ParticleTuple<D> &p) {
  for (unsigned int i=0; i< D; ++i) {
    if (!p[i]) return false;
  }
  return true;
}

inline bool is_dirty(Particle *p) {
  return !p->get_is_changed();
}
template <unsigned int D>
bool is_dirty(const ParticleTuple<D> &p) {
  for (unsigned int i=0; i< D; ++i) {
    if (p[i]->get_is_changed()) return true;
  }
  return false;
}

inline Particle* prechange(Particle*p) {
  return p->get_prechange_particle();
}

template <unsigned int D>
ParticleTuple<D> prechange(const ParticleTuple<D> &p) {
  ParticleTuple<D> ret;
  for (unsigned int i=0; i< D; ++i) {
    ret[i]= p[i]->get_prechange_particle();
  }
  return ret;
}

inline bool is_inactive(const Particle *p) {
  return !p->get_is_active();
}
template <unsigned int D>
bool is_inactive(const ParticleTuple<D> &p) {
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
ParticlesTemp flatten(const VT &in) {
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

inline const ParticlesTemp& flatten(const Particles &in) {
  return in;
}



inline const Particle& streamable(Particle *p) {
  return *p;
}

template <unsigned int D>
inline const ParticleTuple<D>& streamable(const ParticleTuple<D> &p) {
  return p;
}

template <class C, class F>
ParticlesList get_interacting_particles(C *sc,
                                        F *f) {
  ParticlesList ret;
  for (unsigned int i=0; i< sc->get_number(); ++i) {
    ParticlesList t= f->get_interacting_particles(sc->get(i));
    if (!t.empty()) {
      ret.push_back(get_union(t));
    }
  }
  return ret;
}


template <class C, class F>
ParticlesTemp get_input_particles(C *sc,
                                  F *f) {
  ParticlesTemp ret;
  for (unsigned int i=0; i< sc->get_number(); ++i) {
    ParticlesTemp t= f->get_input_particles(sc->get(i));
    ret.insert(ret.end(), t.begin(), t.end());
  }
  return ret;
}


template <class C, class F>
ContainersTemp get_input_containers(C *sc,
                                  F *f) {
  ContainersTemp ret;
  for (unsigned int i=0; i< sc->get_number(); ++i) {
    ContainersTemp t= f->get_input_containers(sc->get(i));
    ret.insert(ret.end(), t.begin(), t.end());
  }
  return ret;
}



template <class C, class F>
ParticlesTemp get_output_particles(C *sc,
                                   F *f) {
  ParticlesTemp ret;
  for (unsigned int i=0; i< sc->get_number(); ++i) {
    ParticlesTemp t= f->get_output_particles(sc->get(i));
    ret.insert(ret.end(), t.begin(), t.end());
  }
  return ret;
}


inline Model *get_model(Particle*p) {
  return p->get_model();
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


#define IMP_ACTIVE_CONTAINER_DECL(Name)                         \
  private:                                                      \
  class Ticker: public ScoreState {                             \
    Name *back_;                                                \
  public:                                                       \
  Ticker(Name *n): ScoreState(n->get_name()+" updater"),        \
                   back_(n){}                                   \
  IMP_SCORE_STATE(Ticker, get_module_version_info());           \
  };                                                            \
  friend class Ticker;                                          \
  ScoreStatePointer ticker_;                                    \
  void do_before_evaluate();                                    \
  void do_after_evaluate();                                     \
  void set_model(Model *m);                                     \
  bool get_has_model() const { return ticker_.get_is_set();}    \
  ParticlesTemp get_state_input_particles() const;              \
  ContainersTemp get_state_input_containers() const;            \

#define IMP_ACTIVE_CONTAINER_DEF(Name)                                  \
  void Name::Ticker::do_before_evaluate() {                             \
    back_->do_before_evaluate();                                        \
  }                                                                     \
  void Name::Ticker::do_after_evaluate(DerivativeAccumulator*) {        \
    back_->do_after_evaluate();                                         \
  }                                                                     \
  ContainersTemp Name::Ticker::get_input_containers() const {           \
    return back_->get_state_input_containers();                        \
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
  ParticlesList Name::Ticker::get_interacting_particles() const {       \
    return ParticlesList();                                             \
  }                                                                     \
  void Name::Ticker::show(std::ostream &out) const {                    \
    out << "Ticker\n";                                                  \
  }                                                                     \
  void Name::set_model(Model *m) {                                      \
    IMP_INTERNAL_CHECK(!get_has_model(),                                \
                       "Can only set the model once.");                 \
    IMP_LOG(TERSE, "Setting up score state for container "              \
            << get_name()<< std::endl);                                 \
    IMP_INTERNAL_CHECK(!get_is_added_or_removed_container(),            \
                       "No active updating of add/remove containers."); \
    ticker_.set(new Ticker(this), m);                                   \
  }








#define IMP_PAIR_CONTAINER_METHODS_FROM_FOREACH(Name)                   \
  void Name::apply(const PairModifier *sm) {                            \
    FOREACH(sm->apply(p));                                              \
  }                                                                     \
  void Name::apply(const PairModifier *sm,                              \
                   DerivativeAccumulator &da) {                         \
    FOREACH(sm->apply(p, da));                                          \
  }                                                                     \
  double Name::evaluate(const PairScore *s,                             \
                        DerivativeAccumulator *da) const {              \
    double score=0;                                                     \
    FOREACH( score+=s->evaluate(p, da));                                \
    return score;                                                       \
  }                                                                     \
  double Name::evaluate_change(const PairScore *s,                      \
                               DerivativeAccumulator *da) const {       \
    double score=0;                                                     \
    FOREACH(score+=s->evaluate_change(p, da));                          \
    return score;                                                       \
  }                                                                     \
  double Name::evaluate_prechange(const PairScore *s,                   \
                                  DerivativeAccumulator *da) const {    \
    double score=0;                                                     \
    FOREACH(score+=s->evaluate_prechange(p, da));                       \
    return score;                                                       \
  }                                                                     \
  ParticlePairsTemp Name::get_particle_pairs() const {                  \
    ParticlePairsTemp ret;                                              \
    ret.reserve(Name::get_number_of_particle_pairs());                  \
    FOREACH(ret.push_back(p));                                          \
    return ret;                                                         \
  }                                                                     \


#define IMP_SINGLETON_CONTAINER_METHODS_FROM_FOREACH(Name)              \
  void Name::apply(const SingletonModifier *sm) {                       \
    FOREACH(sm->apply(a));                                              \
  }                                                                     \
  void Name::apply(const SingletonModifier *sm,                         \
                   DerivativeAccumulator &da) {                         \
    FOREACH(sm->apply(a, da));                                          \
  }                                                                     \
  double Name::evaluate(const SingletonScore *s,                        \
                        DerivativeAccumulator *da) const {              \
    double score=0;                                                     \
    FOREACH( score+=s->evaluate(a, da));                                \
    return score;                                                       \
  }                                                                     \
  double Name::evaluate_change(const SingletonScore *s,                 \
                               DerivativeAccumulator *da) const {       \
    double score=0;                                                     \
    FOREACH(score+=s->evaluate_change(a, da));                          \
    return score;                                                       \
  }                                                                     \
  double Name::evaluate_prechange(const SingletonScore *s,              \
                                  DerivativeAccumulator *da) const {    \
    double score=0;                                                     \
    FOREACH(score+=s->evaluate_prechange(a, da));                       \
    return score;                                                       \
  }                                                                     \
  ParticlesTemp Name::get_particles() const {                           \
    ParticlesTemp ret;                                                  \
    ret.reserve(Name::get_number_of_particles());                       \
    FOREACH(ret.push_back(a));                                          \
    return ret;                                                         \
  }                                                                     \





IMP_END_INTERNAL_NAMESPACE

#endif /* IMP_INTERNAL_CONTAINER_HELPERS_H */
