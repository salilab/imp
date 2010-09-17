/**
 *  \file internal/swig.cpp
 *  \brief Functions for use in swig wrappers
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */
#include <IMP/internal/swig.h>

IMP_BEGIN_INTERNAL_NAMESPACE


double _ConstRestraint
::unprotected_evaluate(IMP::DerivativeAccumulator *) const {
  return v_;
}
void _ConstRestraint::do_show(std::ostream &out) const {
  out << "value: " << v_ << std::endl;
}
ContainersTemp _ConstRestraint::get_input_containers() const {
  return ContainersTemp();
}
ParticlesTemp _ConstRestraint::get_input_particles() const {
  return ps_;
}


double _ConstSingletonScore::evaluate(Particle *,
                                      IMP::DerivativeAccumulator *) const {
  return v_;
}
void _ConstSingletonScore::do_show(std::ostream &out) const {
  out << "value: " << v_ << std::endl;
}
ContainersTemp
_ConstSingletonScore::get_input_containers(Particle *) const {
  return ContainersTemp();
}
ParticlesTemp _ConstSingletonScore::get_input_particles(Particle *p) const {
  return ParticlesTemp(1,p);
}
bool _ConstSingletonScore::get_is_changed(Particle *) const {return false;}




double _ConstPairScore::evaluate(const ParticlePair &,
                                 IMP::DerivativeAccumulator *) const {
  return v_;
}
void _ConstPairScore::do_show(std::ostream &out) const {
  out << "value: " << v_ << std::endl;
}
ContainersTemp
_ConstPairScore::get_input_containers(Particle*p) const {
  return ContainersTemp(1,p);
}
ParticlesTemp _ConstPairScore::get_input_particles(Particle *p) const {
  return ParticlesTemp(1,p);
}
bool _ConstPairScore::get_is_changed(const ParticlePair &) const {return false;}



void _TrivialDecorator::show(std::ostream &out) const {
  out << "trivial decorator " << get_particle()->get_name();
}
void _TrivialDerivedDecorator::show(std::ostream &out) const {
  out << "trivial derived decorator " << get_particle()->get_name();
}

void _TrivialTraitsDecorator::show(std::ostream &out) const {
  out << "trivial traits decorator "<< get_particle()->get_name()
      << " with " << get_decorator_traits();
}



double _ConstOptimizer::optimize(unsigned int n) {
  for (unsigned int i=0; i < n; ++i) {
    get_model()->evaluate(false);
    update_states();
  }
  return get_model()->evaluate(false);
}

void _ConstOptimizer::do_show(std::ostream &out) const {
}

int _overloaded_decorator(_TrivialDecorator) {
  return 0;
}
int _overloaded_decorator(_TrivialDerivedDecorator) {
  return 1;
}



std::string _test_ifile(TextInput a) {
  std::string read;
  while (true) {
    std::string cur;
    a.get_stream() >> cur;
    if (!a) break;
    read= read+cur;
  }
  std::cout << read;
  return read;
}
std::string _test_ofile(TextOutput a) {
  static_cast<std::ostream &>(a) << "hi\n"
                                 << " there, how are things"<< std::endl;
  return "hi\n";
}



std::string _test_ifile_overloaded(TextInput a, std::string s) {
  return _test_ifile(a);
}
std::string _test_ofile_overloaded(TextOutput a, std::string s) {
  return _test_ofile(a);
}
std::string _test_ifile_overloaded(TextInput a, int s) {
  return _test_ifile(a);
}
std::string _test_ofile_overloaded(TextOutput a, int s) {
  return _test_ofile(a);
}



void _decorator_test(Particle*p) {
  std::cout << "hi " << p->get_name() << std::endl;
}

unsigned int _take_particles(const Particles &ps) {
  for (unsigned int i=0; i< ps.size(); ++i) {
    IMP_CHECK_OBJECT(ps[i]);
  }
  return ps.size();
}

unsigned int _take_particles(Model *, const Particles &ps) {
  for (unsigned int i=0; i< ps.size(); ++i) {
    IMP_CHECK_OBJECT(ps[i]);
  }
  return ps.size();
}

unsigned int _take_particles(Model *, const Particles &ps, TextOutput) {
  for (unsigned int i=0; i< ps.size(); ++i) {
    IMP_CHECK_OBJECT(ps[i]);
  }
  return ps.size();
}
const Particles& _give_particles(Model *m) {
  static Particles ret;
  while(ret.size() <10) {
    ret.push_back(new Particle(m));
  }
  return ret;
}
const Particles& _pass_particles(const Particles &ps) {
  return ps;
}
Particle* _pass_particle(Particle* ps) {
  return ps;
}
const ParticlePair& _pass_particle_pair(const ParticlePair &pp) {
  for (unsigned int i=0; i< 2; ++i) {
    std::cout << pp[i]->get_name() << " ";
  }
  std::cout << std::endl;
  return pp;
}
Particles _give_particles_copy(Model *m) {
  Particles ret;
  while(ret.size() <10) {
    ret.push_back(new Particle(m));
  }
  return ret;
}
FloatKeys _pass_float_keys(const FloatKeys& in) {
  for (unsigned int i=0; i< in.size(); ++i) {
    std::cout << in[i] << " ";
  }
  return in;
}
Floats _pass_floats(const Floats& in) {
  for (unsigned int i=0; i< in.size(); ++i) {
    std::cout << in[i] << " ";
  }
  return in;
}
Ints _pass_ints( Ints in) {
  for (unsigned int i=0; i< in.size(); ++i) {
    std::cout << in[i] << " ";
  }
  return in;
}
const Strings& _pass_strings(const Strings& in) {
  for (unsigned int i=0; i< in.size(); ++i) {
    std::cout << in[i] << " ";
  }
  return in;
}

const Particles &_pass(const Particles &p) {
  for (unsigned int i=0; i< p.size(); ++i) {
    std::cout << p[i]->get_name() << " ";
  }
  return p;
}
const Restraints &_pass(const Restraints &p) {
  for (unsigned int i=0; i< p.size(); ++i) {
    std::cout << p[i]->get_name() << " ";
  }
  return p;
}

const _TrivialDecorators &
_pass_decorators(const internal::_TrivialDecorators &p) {
  for (unsigned int i=0; i< p.size(); ++i) {
    std::cout << p[i] << " ";
  }
  std::cout << std::endl;
  return p;
}

const _TrivialTraitsDecorators &
_pass_decorator_traits(const _TrivialTraitsDecorators &p) {
  for (unsigned int i=0; i< p.size(); ++i) {
    std::cout << p[i] << " ";
  }
  std::cout << std::endl;
  return p;
}

const ParticlePairsTemp &
_pass_particle_pairs(const ParticlePairs &p) {
  for (unsigned int i=0; i< p.size(); ++i) {
    std::cout << p[i] << " ";
  }
  std::cout << std::endl;
  return p;
}


DerivativePair
_pass_pair(const DerivativePair &p) {
  std::cout << p.first << " " << p.second << std::endl;
  return p;
}

std::pair<double, double>
_pass_plain_pair(std::pair<double, double> p) {
  std::cout << p.first << " " << p.second << std::endl;
  return p;
}


int _test_overload(const Particles &) {
  return 0;
}

int _test_overload(const Restraints &) {
  return 1;
}

int _test_intranges(const IntRanges &ips) {
  return ips.size();
}


IMP_END_INTERNAL_NAMESPACE
