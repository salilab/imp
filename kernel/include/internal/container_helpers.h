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
  static Float evaluate_change(const SingletonScore *ss,
                               Particle *p,
                               DerivativeAccumulator *ac) {
    return ss->evaluate_change(p, ac);
  }
  template <class SM>
  static void apply(const SM *ss,
                    Particle *p) {
    ss->apply(p);
  }
  template <class SM>
  static void apply(const SM *ss,
                    Particle *p,
                    DerivativeAccumulator da) {
    ss->apply(p, da);
  }
  static bool is_inactive(const Particle* p) {
    return !p->get_is_active();
  }
  struct IsInactive{
    bool operator()(const Particle* p) const {
      return is_inactive(p);
    }
  };
  static bool is_dirty(const Particle *p) {
    return p->get_is_changed();
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
  static Float evaluate_change(const PairScore *ss,
                               ParticlePair p,
                               DerivativeAccumulator *ac) {
    return ss->evaluate_change(p.first, p.second, ac);
  }
  template <class PM>
  static void apply(const PM *ss,
                    const ParticlePair &p) {
    ss->apply(p.first, p.second);
  }
  template <class PM>
  static void apply(const PM *ss,
                    const ParticlePair &p,
                    DerivativeAccumulator da) {
    ss->apply(p.first, p.second, da);
  }
  static bool is_inactive(const ParticlePair &p) {
    return !p[0]->get_is_active() || !p[1]->get_is_active();
  }
  struct IsInactive{
    bool operator()(const ParticlePair &p) const {
      return is_inactive(p);
    }
  };
  static bool is_dirty(const ParticlePair &p) {
    return p[0]->get_is_changed()
      || p[1]->get_is_changed();
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
ParticlesTemp get_input_particles(SingletonContainer *sc,
                                  F *f) {
  ParticlesTemp ret;
  for (unsigned int i=0; i< sc->get_number_of_particles(); ++i) {
    ParticlesTemp t= f->get_input_particles(sc->get_particle(i));
    ret.insert(ret.end(), t.begin(), t.end());
  }
  return ret;
}

template <class F>
ParticlesTemp get_input_particles(PairContainer *sc,
                                  F *f) {
  ParticlesTemp ret;
  for (unsigned int i=0; i< sc->get_number_of_particle_pairs(); ++i) {
    ParticlePair pp=sc->get_particle_pair(i);
    ParticlesTemp t= f->get_input_particles(pp[0], pp[1]);
    ret.insert(ret.end(), t.begin(), t.end());
  }
  return ret;
}


template <class F>
ParticlesTemp get_input_particles(Particle *p,
                                  F *f) {
  ParticlesTemp t= f->get_input_particles(p);
  return t;
}




template <class F>
ParticlesTemp get_input_particles(ParticlePair p,
                                  F *f) {
  ParticlesTemp t= f->get_input_particles(p[0], p[1]);
  return t;
}





template <class F>
ParticlesTemp get_output_particles(SingletonContainer *sc,
                                   F *f) {
  ParticlesTemp ret;
  for (unsigned int i=0; i< sc->get_number_of_particles(); ++i) {
    ParticlesTemp t= f->get_output_particles(sc->get_particle(i));
    ret.insert(ret.end(), t.begin(), t.end());
  }
  return ret;
}

template <class F>
ParticlesTemp get_output_particles(PairContainer *sc,
                                   F *f) {
  ParticlesTemp ret;
  for (unsigned int i=0; i< sc->get_number_of_particle_pairs(); ++i) {
    ParticlePair pp=sc->get_particle_pair(i);
    ParticlesTemp t= f->get_output_particles(pp[0], pp[1]);
    ret.insert(ret.end(), t.begin(), t.end());
  }
  return ret;
}


template <class F>
ParticlesTemp get_output_particles(Particle *p,
                                   F *f) {
  ParticlesTemp t= f->get_output_particles(p);
  return t;
}




template <class F>
ParticlesTemp get_output_particles(ParticlePair p,
                                   F *f) {
  ParticlesTemp t= f->get_output_particles(p[0], p[1]);
  return t;
}

inline Model *get_model(Particle*p) {
  return p->get_model();
}

inline Model *get_model(ParticlePair p) {
  return p[0]->get_model();
}

inline std::string get_name(Particle*p) {
  return p->get_name();
}

inline std::string get_name(ParticlePair p) {
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

#define IMP_ACTIVE_CONTAINER_DEF(Name)                                  \
  void Name::Ticker::do_before_evaluate() {                             \
    back_->do_before_evaluate();                                        \
  }                                                                     \
  void Name::Ticker::do_after_evaluate(DerivativeAccumulator*) {         \
    back_->do_after_evaluate();                                         \
  }                                                                     \
  ObjectsTemp Name::Ticker::get_input_objects() const {                 \
    return ObjectsTemp();                                               \
  }                                                                     \
  ObjectsTemp Name::Ticker::get_output_objects() const {                \
    return ObjectsTemp(1, back_);                                       \
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
    FOREACH(sm->apply(a,b));                                            \
  }                                                                     \
  void Name::apply(const PairModifier *sm,                              \
                   DerivativeAccumulator &da) {                         \
    FOREACH(sm->apply(a,b, da));                                        \
  }                                                                     \
  double Name::evaluate(const PairScore *s,                             \
                        DerivativeAccumulator *da) const {              \
    double score=0;                                                     \
    FOREACH( score+=s->evaluate(a,b, da));                              \
    return score;                                                       \
  }                                                                     \
  double Name::evaluate_change(const PairScore *s,                      \
                               DerivativeAccumulator *da) const {       \
    double score=0;                                                     \
    FOREACH(score+=s->evaluate_change(a, b, da));                       \
    return score;                                                       \
  }                                                                     \
  double Name::evaluate_prechange(const PairScore *s,                   \
                                  DerivativeAccumulator *da) const {    \
    double score=0;                                                     \
    FOREACH(score+=s->evaluate_prechange(a, b, da));                    \
    return score;                                                       \
  }                                                                     \
  ParticlePairsTemp Name::get_particle_pairs() const {                  \
    ParticlePairsTemp ret;                                              \
    ret.reserve(Name::get_number_of_particle_pairs());                  \
    FOREACH(ret.push_back(ParticlePair(a,b)));                          \
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
