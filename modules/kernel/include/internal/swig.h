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
#include "../file.h"
#include "../Optimizer.h"
#include "container_helpers.h"
#include <IMP/base/map.h>
#include <IMP/base/internal/swig.h>
#include <IMP/base/deprecation_macros.h>


IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

// these can't go %inline since swig won't wrap them in other modules but will
// try to build converters.
// probably not legal C++, but for python
class IMPKERNELEXPORT _ConstRestraint: public Restraint {
  double v_;
  const ParticlesTemp ps_;
public:
  _ConstRestraint(double v,
                  const ParticlesTemp ps):
      Restraint(internal::get_model(ps), "ConstRestraint%1%"), v_(v),
      ps_(ps){}
  _ConstRestraint(double v):
      Restraint("ConstRestraint%1%"), v_(v){}
  _ConstRestraint(Model *m, const ParticleIndexes &pis,
                  double v): Restraint(m, "ConstRestraint%1%"), v_(v),
      ps_(get_particles(m, pis)) {}
  double get_value() const {return v_;}
  Restraints do_create_decomposition() const;
  IMP_RESTRAINT(_ConstRestraint);
};

IMP_OBJECTS(_ConstRestraint, _ConstRestraints);

class IMPKERNELEXPORT _ConstSingletonScore: public SingletonScore {
  double v_;
public:
  _ConstSingletonScore(double v): v_(v){}
  IMP_SINGLETON_SCORE(_ConstSingletonScore);
};
IMP_OBJECTS(_ConstSingletonScore, _ConstSingletonScores);



class IMPKERNELEXPORT _ConstPairScore: public PairScore {
  double v_;
public:
  _ConstPairScore(double v): v_(v){}
  IMP_PAIR_SCORE(_ConstPairScore);
};
IMP_OBJECTS(_ConstPairScore, _ConstPairScores);


class IMPKERNELEXPORT _TrivialDecorator: public Decorator {
public:
  IMP_DECORATOR(_TrivialDecorator, Decorator);
  static _TrivialDecorator setup_particle(Particle *p) {
    p->add_attribute(IntKey("trivial_attribute"), 1);
    return _TrivialDecorator(p);
  }
  static bool particle_is_instance(Particle *p) {
    return p->has_attribute(IntKey("trivial_attribute"));
  }
};

class IMPKERNELEXPORT _TrivialDerivedDecorator: public _TrivialDecorator {
public:
  IMP_DECORATOR(_TrivialDerivedDecorator, _TrivialDecorator);
  static _TrivialDerivedDecorator setup_particle(Particle *p) {
    p->add_attribute(IntKey("trivial_attribute_2"), 2);
    _TrivialDecorator::setup_particle(p);
    return _TrivialDerivedDecorator(p);
  }
  static bool particle_is_instance(Particle *p) {
    return p->has_attribute(IntKey("trivial_attribute_2"));
  }
};

IMP_DECORATORS(_TrivialDecorator, _TrivialDecorators, ParticlesTemp);
IMP_DECORATORS(_TrivialDerivedDecorator,
               _TrivialDerivedDecorators, _TrivialDecorators);


class IMPKERNELEXPORT _TrivialTraitsDecorator:
public Decorator
{
public:
  IMP_DECORATOR_WITH_TRAITS(_TrivialTraitsDecorator, Decorator,
                       StringKey, sk,
                       get_default_key());
  static _TrivialTraitsDecorator setup_particle(Particle *p,
                                                StringKey k=get_default_key()) {
    p->add_attribute(k, "hi");
    return _TrivialTraitsDecorator(p, k);
  }
  static bool particle_is_instance(Particle *p,
                                   StringKey k=get_default_key()) {
    return p->has_attribute(k);
  }
  static StringKey get_default_key() {
    return StringKey("traits dec");
  }
};

IMP_DECORATORS_WITH_TRAITS(_TrivialTraitsDecorator,
                           _TrivialTraitsDecorators, Particles);





class IMPKERNELEXPORT _ConstOptimizer: public Optimizer {
 public:
  _ConstOptimizer(Model *m): Optimizer(m, "ConstOptimizer%1%"){}
  IMP_OPTIMIZER(_ConstOptimizer);
};

IMP_OBJECTS(_ConstOptimizer, _ConstOptimizers);




inline Particle *get_particle(Particle *p) {
   return p;
}

IMPKERNELEXPORT void _decorator_test(Particle*p);
IMPKERNELEXPORT int _overloaded_decorator(_TrivialDecorator a);
IMPKERNELEXPORT int _overloaded_decorator(_TrivialDerivedDecorator a);

IMPKERNELEXPORT unsigned int _take_particles(const Particles &ps);

IMPKERNELEXPORT unsigned int _take_particles(Model *m, const Particles &ps);

IMPKERNELEXPORT unsigned int _take_particles(Model *m,
                                       const Particles &ps,
                                       base::TextOutput out);
IMPKERNELEXPORT const Particles& _give_particles(Model *m);
IMPKERNELEXPORT const Particles& _pass_particles(const Particles &ps);
IMPKERNELEXPORT Particle* _pass_particle(Particle* ps);
IMPKERNELEXPORT const ParticlePair& _pass_particle_pair(const ParticlePair &pp);
IMPKERNELEXPORT Particles _give_particles_copy(Model *m);
IMPKERNELEXPORT FloatKeys _pass_float_keys(const FloatKeys& input);

IMPKERNELEXPORT const Particles &_pass(const Particles &p);
IMPKERNELEXPORT const Restraints &_pass(const Restraints &p);
IMPKERNELEXPORT const _TrivialDecorators &
_pass_decorators(const internal::_TrivialDecorators &p);

IMPKERNELEXPORT const _TrivialTraitsDecorators &
_pass_decorator_traits(const _TrivialTraitsDecorators &p);

IMPKERNELEXPORT ParticlePairsTemp
_pass_particle_pairs(const ParticlePairsTemp &p);

IMPKERNELEXPORT ParticleIndexPairs
_pass_particle_index_pairs(const ParticleIndexPairs &p);


IMPKERNELEXPORT ModelObjectsTemp
_pass_model_objects(const ModelObjectsTemp &p);


inline ParticlesTemps
_pass_particles_temps(const ParticlesTemps &ps) {
  return ps;
}


IMPKERNELEXPORT int _test_overload(const Particles &ps);

IMPKERNELEXPORT int _test_overload(const Restraints &ps);



#if 0
inline void bad_pass(FloatKey*) {}
inline void bad_pass(FloatKeys*) {}
#endif

inline FloatRange _get_range(Model *m,
                             FloatKey k) {
  return m->get_range(k);
}


IMPKERNELEXPORT ParticlesTemp
_create_particles_from_pdb(std::string name, Model*m);




//! Track the pairs of particles passed.
/** Primarily for testing.
 */
class IMPKERNELEXPORT _LogPairScore : public PairScore
{
  mutable base::map<ParticlePair, unsigned int> map_;
 public:
  //! create with an empty map
  _LogPairScore(){}
  IMP_SIMPLE_PAIR_SCORE(_LogPairScore);

  //! Get a list of all pairs (without multiplicity)
  ParticlePairsTemp get_particle_pairs() const ;
  //! Clear the lst of pairs
  void clear() {
    map_.clear();
  }
  //! Return true if the pair is in the list
  bool get_contains(const ParticlePair &pp) const {
    return map_.find(pp) != map_.end();
  }
};


#if IMP_HAS_DEPRECATED
class PythonRestraint: public Restraint {
public:
  PythonRestraint(Model *m, std::string name="PythonRestraint%1%"):
    Restraint(m, name) {}
  PythonRestraint(std::string name="PythonRestraint%1%"):
    Restraint(name) {}
 virtual ParticlesTemp get_input_particles() const=0;
 virtual ContainersTemp get_input_containers() const=0;

ModelObjectsTemp do_get_inputs() const {
  ModelObjectsTemp ret;
  ret+= get_input_particles();
  ret+= get_input_containers();
  return ret;
}
};
IMP_OBJECTS(PythonRestraint, PythonRestraints);

class PythonScoreState: public ScoreState {
public:
  PythonScoreState(std::string name="PythonScoreState%1%"):
    ScoreState(name) {}
  PythonScoreState(Model *m, std::string name="PythonScoreState%1%"):
    ScoreState(m, name) {}
  virtual ParticlesTemp get_input_particles() const=0;
  virtual ContainersTemp get_input_containers() const=0;
  virtual ParticlesTemp get_output_particles() const=0;
  virtual ContainersTemp get_output_containers() const=0;

  ModelObjectsTemp do_get_inputs() const {
    ModelObjectsTemp ret;
    ret+= get_input_particles();
    ret+= get_input_containers();
    return ret;
  }
  ModelObjectsTemp do_get_outputs() const {
    ModelObjectsTemp ret;
    ret+= get_output_particles();
    ret+= get_output_containers();
    return ret;
  }
};
IMP_OBJECTS(PythonScoreState, PythonScoreStates);
#endif

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
inline void _implicit_particles(const _ImplicitParticles&) {}

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_SWIG_H */
