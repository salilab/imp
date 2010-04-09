/**
 *  \file internal/swig.h
 *  \brief Functions for use in swig wrappers
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMP_INTERNAL_SWIG_H
#define IMP_INTERNAL_SWIG_H

#include "../kernel_config.h"
#include "../Particle.h"
#include "../ParticleTuple.h"
#include "../Restraint.h"
#include "../SingletonScore.h"
#include "../macros.h"
#include "../Decorator.h"
#include "../file.h"

IMP_BEGIN_INTERNAL_NAMESPACE

// these can't go %inline since swig won't wrap them in other modules but will
// try to build converters.
// probably not legal C++, but for python
class IMPEXPORT _ConstRestraint: public IMP::Restraint {
  double v_;
public:
  _ConstRestraint(double v): v_(v){}
  IMP_RESTRAINT(_ConstRestraint);
};



class IMPEXPORT _ConstSingletonScore: public IMP::SingletonScore {
  double v_;
public:
  _ConstSingletonScore(double v): v_(v){}
  IMP_SINGLETON_SCORE(_ConstSingletonScore);
};



class IMPEXPORT _ConstPairScore: public IMP::PairScore {
  double v_;
public:
  _ConstPairScore(double v): v_(v){}
  IMP_PAIR_SCORE(_ConstPairScore);
};


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

IMP_DECORATORS(_TrivialDecorator, _TrivialDecorators, Particles);
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
IMP_OUTPUT_OPERATOR(_TrivialDecorator);
IMP_OUTPUT_OPERATOR(_TrivialDerivedDecorator);
IMP_OUTPUT_OPERATOR(_TrivialTraitsDecorator);
















IMPEXPORT std::string _test_ifile(TextInput a);
IMPEXPORT std::string _test_ofile(TextOutput a);

IMPEXPORT void _decorator_test(Particle*p);
IMPEXPORT int _overloaded_decorator(_TrivialDecorator a);
IMPEXPORT int _overloaded_decorator(_TrivialDerivedDecorator a);

IMPEXPORT unsigned int _take_particles(const Particles &ps);

IMPEXPORT unsigned int _take_particles(Model *m, const Particles &ps);

IMPEXPORT unsigned int _take_particles(Model *m,
                                       const Particles &ps, TextOutput out);
IMPEXPORT const Particles& _give_particles(Model *m);
IMPEXPORT const Particles& _pass_particles(const Particles &ps);
IMPEXPORT Particle* _pass_particle(Particle* ps);
IMPEXPORT const ParticlePair& _pass_particle_pair(const ParticlePair &pp);
IMPEXPORT Particles _give_particles_copy(Model *m);
IMPEXPORT FloatKeys _pass_float_keys(const FloatKeys& in);
IMPEXPORT Floats _pass_floats(const Floats& in);
IMPEXPORT Ints _pass_ints( Ints in);
IMPEXPORT const Strings& _pass_strings(const Strings& in);

IMPEXPORT const Particles &_pass(const Particles &p);
IMPEXPORT const Restraints &_pass(const Restraints &p);
IMPEXPORT const _TrivialDecorators &
_pass_decorators(const internal::_TrivialDecorators &p);

IMPEXPORT const _TrivialTraitsDecorators &
_pass_decorator_traits(const _TrivialTraitsDecorators &p);

IMPEXPORT const ParticlePairsTemp &
_pass_particle_pairs(const ParticlePairs &p);


IMPEXPORT DerivativePair
_pass_pair(const DerivativePair &p);


IMPEXPORT int _test_overload(const Particles &ps);

IMPEXPORT int _test_overload(const Restraints &ps);


IMP_END_INTERNAL_NAMESPACE

#endif  /* IMP_INTERNAL_SWIG_H */
