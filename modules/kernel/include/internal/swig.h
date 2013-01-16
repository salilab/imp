/**
 *  \file internal/swig.h
 *  \brief Functions for use in swig wrappers
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_SWIG_H
#define IMPKERNEL_INTERNAL_SWIG_H

#include <IMP/kernel_config.h>
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
#include <IMP/compatibility/map.h>
#include <IMP/base/internal/swig.h>
#include <IMP/base/deprecation_macros.h>


IMP_BEGIN_INTERNAL_NAMESPACE

// these can't go %inline since swig won't wrap them in other modules but will
// try to build converters.
// probably not legal C++, but for python
class IMPEXPORT _ConstRestraint: public IMP::Restraint {
  double v_;
  const ParticlesTemp ps_;
public:
  _ConstRestraint(double v,
                  const ParticlesTemp ps):
      Restraint(internal::get_model(ps), "ConstRestraint%1%"), v_(v),
      ps_(ps){}
  _ConstRestraint(double v):
      Restraint("ConstRestraint%1%"), v_(v){}
  double get_value() const {return v_;}
  Restraints do_create_decomposition() const;
  IMP_RESTRAINT(_ConstRestraint);
};

IMP_OBJECTS(_ConstRestraint, _ConstRestraints);

class IMPEXPORT _ConstSingletonScore: public IMP::SingletonScore {
  double v_;
public:
  _ConstSingletonScore(double v): v_(v){}
  IMP_SINGLETON_SCORE(_ConstSingletonScore);
};
IMP_OBJECTS(_ConstSingletonScore, _ConstSingletonScores);



class IMPEXPORT _ConstPairScore: public IMP::PairScore {
  double v_;
public:
  _ConstPairScore(double v): v_(v){}
  IMP_PAIR_SCORE(_ConstPairScore);
};
IMP_OBJECTS(_ConstPairScore, _ConstPairScores);


class IMPEXPORT _TrivialDecorator: public Decorator {
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

class IMPEXPORT _TrivialDerivedDecorator: public _TrivialDecorator {
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


class IMPEXPORT _TrivialTraitsDecorator:
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





class IMPEXPORT _ConstOptimizer: public Optimizer {
 public:
  _ConstOptimizer(Model *m): Optimizer(m, "ConstOptimizer%1%"){}
  IMP_OPTIMIZER(_ConstOptimizer);
};

IMP_OBJECTS(_ConstOptimizer, _ConstOptimizers);


class IMPEXPORT _Value {
  int i_;
 public:
  _Value(int i): i_(i){}
  IMP_SHOWABLE_INLINE(_Value, out << i_;);
  IMP_COMPARISONS_1(_Value, i_);
  int get() const {return i_;}
};

IMP_VALUES(_Value, _Values);







inline IMP::Particle *get_particle(IMP::Particle *p) {
   return p;
}

IMPEXPORT std::string _test_ifile(base::TextInput a);
IMPEXPORT std::string _test_ofile(base::TextOutput a);
// overload
IMPEXPORT std::string _test_ifile_overloaded(base::TextInput a, int i);
IMPEXPORT std::string _test_ofile_overloaded(base::TextOutput a, int i);
IMPEXPORT std::string _test_ifile_overloaded(base::TextInput a, std::string st);
IMPEXPORT std::string _test_ofile_overloaded(base::TextOutput a,
                                             std::string st);


IMPEXPORT void _decorator_test(Particle*p);
IMPEXPORT int _overloaded_decorator(_TrivialDecorator a);
IMPEXPORT int _overloaded_decorator(_TrivialDerivedDecorator a);

IMPEXPORT unsigned int _take_particles(const Particles &ps);

IMPEXPORT unsigned int _take_particles(Model *m, const Particles &ps);

IMPEXPORT unsigned int _take_particles(Model *m,
                                       const Particles &ps,
                                       base::TextOutput out);
IMPEXPORT const Particles& _give_particles(Model *m);
IMPEXPORT const Particles& _pass_particles(const Particles &ps);
IMPEXPORT Particle* _pass_particle(Particle* ps);
IMPEXPORT const ParticlePair& _pass_particle_pair(const ParticlePair &pp);
IMPEXPORT Particles _give_particles_copy(Model *m);
IMPEXPORT FloatKeys _pass_float_keys(const FloatKeys& input);
IMPEXPORT Floats _pass_floats(const Floats& input);
IMPEXPORT Ints _pass_ints( Ints input);
IMPEXPORT IntsList _pass_ints_list(const IntsList &input);
IMPEXPORT IntsLists _pass_ints_lists(const IntsLists &input);
IMPEXPORT const Strings& _pass_strings(const Strings& input);

IMPEXPORT const Particles &_pass(const Particles &p);
IMPEXPORT const Restraints &_pass(const Restraints &p);
IMPEXPORT const _TrivialDecorators &
_pass_decorators(const internal::_TrivialDecorators &p);

IMPEXPORT const _TrivialTraitsDecorators &
_pass_decorator_traits(const _TrivialTraitsDecorators &p);

IMPEXPORT ParticlePairsTemp
_pass_particle_pairs(const ParticlePairsTemp &p);

IMPEXPORT ParticleIndexPairs
_pass_particle_index_pairs(const ParticleIndexPairs &p);


IMPEXPORT ModelObjectsTemp
_pass_model_objects(const ModelObjectsTemp &p);


IMPEXPORT DerivativePair
_pass_pair(const DerivativePair &p);

IMPEXPORT FloatPair
_pass_plain_pair( FloatPair p);

inline ParticlesTemps
_pass_particles_temps(const ParticlesTemps &ps) {
  return ps;
}


IMPEXPORT int _test_overload(const Particles &ps);

IMPEXPORT int _test_overload(const Restraints &ps);

IMPEXPORT int _test_intranges(const IntRanges &ips);

IMPEXPORT IntRange _test_intrange(const IntRange &ips);

// for overload
IMPEXPORT IntRange _test_intrange();

#if 0
inline void bad_pass(FloatKey*) {}
inline void bad_pass(FloatKeys*) {}
#endif

inline FloatRange _get_range(Model *m,
                             FloatKey k) {
  return m->get_range(k);
}


IMPEXPORT ParticlesTemp _create_particles_from_pdb(std::string name, Model*m);




//! Track the pairs of particles passed.
/** Primarily for testing.
 */
class IMPEXPORT _LogPairScore : public PairScore
{
  mutable compatibility::map<ParticlePair, unsigned int> map_;
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


#ifdef IMP_USE_DEPRECATED
class PythonRestraint: public IMP::Restraint {
public:
PythonRestraint(std::string name="PythonRestraint%1%"): IMP::Restraint(name) {}
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

class PythonScoreState: public IMP::ScoreState {
public:
  PythonScoreState(std::string name="PythonScoreState%1%"):
    IMP::ScoreState(name) {}
  PythonScoreState(Model *m, std::string name="PythonScoreState%1%"):
    IMP::ScoreState(m, name) {}
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

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_SWIG_H */
