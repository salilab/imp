/**
 *  \file internal/swig.h
 *  \brief Functions for use in swig wrappers
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_SWIG_H
#define IMPKERNEL_INTERNAL_SWIG_H

#include <IMP/kernel/kernel_config.h>
#include "../Particle.h"
#include "../ParticleTuple.h"
#include "../Restraint.h"
#include "../SingletonScore.h"
#include "../PairScore.h"
#include "../macros.h"
#include "../Decorator.h"
#include "../Optimizer.h"
#include "container_helpers.h"
#include <IMP/base/map.h>
#include <IMP/base/internal/swig.h>
#include <IMP/base/deprecation_macros.h>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

// these can't go %inline since swig won't wrap them in other modules but will
// try to build converters.
// probably not legal C++, but for python
class IMPKERNELEXPORT _ConstRestraint : public Restraint {
  double v_;
  const ParticlesTemp ps_;

 public:
  _ConstRestraint(double v, const ParticlesTemp ps)
      : Restraint(internal::get_model(ps), "ConstRestraint%1%"),
        v_(v),
        ps_(ps) {}
  _ConstRestraint(Model *m, const ParticleIndexes &pis, double v)
      : Restraint(m, "ConstRestraint%1%"), v_(v), ps_(get_particles(m, pis)) {}
  double get_value() const { return v_; }
  Restraints do_create_decomposition() const;
  double unprotected_evaluate(IMP::kernel::DerivativeAccumulator *accum) const
      IMP_OVERRIDE;
  ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(_ConstRestraint);
};

IMP_OBJECTS(_ConstRestraint, _ConstRestraints);

class _ConstSingletonScore : public SingletonScore {
  double v_;

 public:
  _ConstSingletonScore(double v) : v_(v) {}
  virtual double evaluate_index(Model *, ParticleIndex,
                                DerivativeAccumulator *) const IMP_OVERRIDE {
    return v_;
  }
  virtual ModelObjectsTemp do_get_inputs(Model *, const ParticleIndexes &) const
      IMP_OVERRIDE {
    return ModelObjectsTemp();
  }
  IMP_SINGLETON_SCORE_METHODS(_ConstSingletonScore);
  IMP_OBJECT_METHODS(_ConstSingletonScore);
};
IMP_OBJECTS(_ConstSingletonScore, _ConstSingletonScores);

class IMPKERNELEXPORT _ConstPairScore : public PairScore {
  double v_;

 public:
  _ConstPairScore(double v) : v_(v) {}
  virtual double evaluate_index(Model *, const ParticleIndexPair &,
                                DerivativeAccumulator *) const IMP_OVERRIDE {
    return v_;
  }
  virtual ModelObjectsTemp do_get_inputs(Model *, const ParticleIndexes &) const
      IMP_OVERRIDE {
    return ModelObjectsTemp();
  }
  IMP_PAIR_SCORE_METHODS(_ConstPairScore);
  IMP_OBJECT_METHODS(_ConstPairScore);
};
IMP_OBJECTS(_ConstPairScore, _ConstPairScores);

class IMPKERNELEXPORT _TrivialDecorator : public Decorator {
  static void do_setup_particle(Model *m, ParticleIndex pi);

 public:
  IMP_DECORATOR_METHODS(_TrivialDecorator, Decorator);
  IMP_DECORATOR_SETUP_0(_TrivialDecorator);
  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return m->get_has_attribute(IntKey("trivial_attribute"), pi);
  }
};

class IMPKERNELEXPORT _TrivialDerivedDecorator : public _TrivialDecorator {
  static void do_setup_particle(Model *m, ParticleIndex pi);

 public:
  IMP_DECORATOR_METHODS(_TrivialDerivedDecorator, _TrivialDecorator);
  IMP_DECORATOR_SETUP_0(_TrivialDerivedDecorator);
  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return m->get_has_attribute(IntKey("trivial_attribute_2"), pi);
  }
};

IMP_DECORATORS(_TrivialDecorator, _TrivialDecorators, ParticlesTemp);
IMP_DECORATORS(_TrivialDerivedDecorator, _TrivialDerivedDecorators,
               _TrivialDecorators);

class IMPKERNELEXPORT _TrivialTraitsDecorator : public Decorator {
  static void do_setup_particle(Model *m, ParticleIndex pi, StringKey k);

 public:
  IMP_DECORATOR_WITH_TRAITS_METHODS(_TrivialTraitsDecorator, Decorator,
                                    StringKey, sk, get_default_key());
  IMP_DECORATOR_TRAITS_SETUP_0(_TrivialTraitsDecorator);
  static bool get_is_setup(Model *m, ParticleIndex pi,
                           StringKey k = get_default_key()) {
    return m->get_has_attribute(k, pi);
  }
  static StringKey get_default_key() { return StringKey("traits dec"); }
};

IMP_DECORATORS_WITH_TRAITS(_TrivialTraitsDecorator, _TrivialTraitsDecorators,
                           Particles);

class IMPKERNELEXPORT _ConstOptimizer : public Optimizer {
 public:
  _ConstOptimizer(Model *m) : Optimizer(m, "ConstOptimizer%1%") {}
  virtual Float do_optimize(unsigned int max_steps) IMP_OVERRIDE;
  IMP_OBJECT_METHODS(_ConstOptimizer);
};

IMP_OBJECTS(_ConstOptimizer, _ConstOptimizers);

inline Particle *get_particle(Particle *p) { return p; }

IMPKERNELEXPORT void _decorator_test(Particle *p);
IMPKERNELEXPORT int _overloaded_decorator(_TrivialDecorator a);
IMPKERNELEXPORT int _overloaded_decorator(_TrivialDerivedDecorator a);

IMPKERNELEXPORT unsigned int _take_particles(const Particles &ps);

IMPKERNELEXPORT unsigned int _take_particles(Model *m, const Particles &ps);

IMPKERNELEXPORT unsigned int _take_particles(Model *m, const Particles &ps,
                                             base::TextOutput out);
IMPKERNELEXPORT const Particles &_give_particles(Model *m);
IMPKERNELEXPORT const Particles &_pass_particles(const Particles &ps);
IMPKERNELEXPORT Particle *_pass_particle(Particle *ps);
IMPKERNELEXPORT const ParticlePair &_pass_particle_pair(const ParticlePair &pp);
IMPKERNELEXPORT Particles _give_particles_copy(Model *m);
IMPKERNELEXPORT FloatKeys _pass_float_keys(const FloatKeys &input);

IMPKERNELEXPORT const Particles &_pass(const Particles &p);
IMPKERNELEXPORT const Restraints &_pass(const Restraints &p);
IMPKERNELEXPORT const _TrivialDecorators &_pass_decorators(
    const internal::_TrivialDecorators &p);

IMPKERNELEXPORT const _TrivialTraitsDecorators &_pass_decorator_traits(
    const _TrivialTraitsDecorators &p);

IMPKERNELEXPORT ParticlePairsTemp
    _pass_particle_pairs(const ParticlePairsTemp &p);

IMPKERNELEXPORT ParticleIndexPairs
    _pass_particle_index_pairs(const ParticleIndexPairs &p);

IMPKERNELEXPORT ModelObjectsTemp _pass_model_objects(const ModelObjectsTemp &p);

inline ParticlesTemps _pass_particles_temps(const ParticlesTemps &ps) {
  return ps;
}

IMPKERNELEXPORT int _test_overload(const Particles &ps);

IMPKERNELEXPORT int _test_overload(const Restraints &ps);

#if 0
inline void bad_pass(FloatKey *) {}
inline void bad_pass(FloatKeys *) {}
#endif

inline FloatRange _get_range(Model *m, FloatKey k) { return m->get_range(k); }

IMPKERNELEXPORT ParticlesTemp
    _create_particles_from_pdb(std::string name, Model *m);

//! Track the pairs of particles passed.
/** Primarily for testing.
 */
class IMPKERNELEXPORT _LogPairScore : public PairScore {
  mutable base::map<ParticlePair, unsigned int> map_;

 public:
  //! create with an empty map
  _LogPairScore() {}
  virtual double evaluate_index(Model *m, const ParticleIndexPair &p,
                                DerivativeAccumulator *da) const IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs(Model *, const ParticleIndexes &) const
      IMP_OVERRIDE {
    return ModelObjectsTemp();
  }
  IMP_PAIR_SCORE_METHODS(_LogPairScore);
  IMP_OBJECT_METHODS(_LogPairScore);

  //! Get a list of all pairs (without multiplicity)
  ParticlePairsTemp get_particle_pairs() const;
  //! Clear the lst of pairs
  void clear() { map_.clear(); }
  //! Return true if the pair is in the list
  bool get_contains(const ParticlePair &pp) const {
    return map_.find(pp) != map_.end();
  }
};

inline void _overloaded_particles(Particle *) {}
inline void _overloaded_particles(const Particles &) {}
inline void _overloaded_particles(const ParticlesTemp &) {}
inline void _overloaded_particles(Model *, const ParticleIndexes &) {}
inline void _overloaded_particles(const _TrivialDecorators &) {}
inline void _overloaded_particles(_TrivialDecorator) {}

struct _ImplicitParticles {
  _ImplicitParticles(Particle *) {}
  _ImplicitParticles(const Particles &) {}
  _ImplicitParticles(const ParticlesTemp &) {}
  _ImplicitParticles(Model *, const ParticleIndexes &) {}
  _ImplicitParticles(const _TrivialDecorators &) {}
  _ImplicitParticles(_TrivialDecorator) {}
};
inline void _implicit_particles(const _ImplicitParticles &) {}

IMPKERNELEXPORT ParticleIndex _take_particle_adaptor(ParticleAdaptor pa);

IMPKERNELEXPORT ParticleIndexes
    _take_particle_indexes_adaptor(ParticleIndexesAdaptor pa);

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_INTERNAL_SWIG_H */
