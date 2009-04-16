%module(directors="1") IMP

%{
#include "IMP.h"
%}

%include "std_vector.i"
%include "std_map.i"
%include "std_string.i"
%include "std_pair.i"

%include "IMP_macros.i"
%include "IMP_exceptions.i"
%include "IMP_keys.i"

%include "typemaps.i"

%ignore IMP::ParticlePair::operator[];
%ignore IMP::ParticleTriplet::operator[];
%extend IMP::ParticlePair {
  Particle* __getitem__(unsigned int index) const {
    return self->operator[](index);
  }
  void __setitem__(unsigned int index, Particle* val) {
    self->operator[](index) = val;
  }
  int __len__() const {
      return 2;
  }
}
%extend IMP::ParticleTriplet {
  Particle* __getitem__(unsigned int index) const {
    return self->operator[](index);
  }
  void __setitem__(unsigned int index, Particle* val) {
    self->operator[](index) = val;
  }
  int __len__() const {
      return 3;
  }
}  

namespace IMP {
  %typemap(out) std::pair<Float,Float> {
     PyObject *tup= PyTuple_New(2);
     PyTuple_SetItem(tup, 0, PyFloat_FromDouble($1.first));
     PyTuple_SetItem(tup, 1, PyFloat_FromDouble($1.second));
     $result= tup;
  }
}


%pythoncode %{
def check_particle(p, a):
   if (not p.get_is_active()):
      raise ValueError("Inactive Particle")
   if (type(a)() == a):
      raise IndexError("Cannot use default Index")
   if (not p.has_attribute(a)):
      raise IndexError("Particle does not have attribute")
%}

namespace IMP {
  // need to special case particle so can't add this to macro
  %pythonprepend Particle::get_value %{
        check_particle(args[0], args[1])
  %}
  %pythonprepend Particle::get_is_optimized %{
        check_particle(args[0], args[1])
  %}
  %pythonprepend Particle::set_is_optimized %{
        check_particle(args[0], args[1])
  %}
  %pythonprepend Particle::set_value %{
        check_particle(args[0], args[1])
  %}
  %pythonprepend Particle::add_to_derivative %{
        check_particle(args[0], args[1])
  %}
  %pythonprepend Particle::get_derivative %{
        check_particle(args[0], args[1])
  %}
  %pythonprepend Particle::add_attribute %{
        # special case since we don't want to check that the attribute is there
        if (not args[0].get_is_active()):
           raise ValueError("Inactive Particle")
        elif (type(args[1])() == args[1]):
           raise IndexError("Cannot use default Index")
        elif (args[0].has_attribute(args[1])):
           raise IndexError("Particle already has attribute")

  %}

  // special case since particles are ref counted
  %extend Model {
    Particles get_particles() const {
      IMP::Particles ret(self->particles_begin(), self->particles_end());
      return ret;
    }
  }
  IMP_CONTAINER_SWIG(Model, ScoreState, score_state)
  IMP_CONTAINER_SWIG(Model, Restraint, restraint)
  IMP_CONTAINER_SWIG(Optimizer, OptimizerState, optimizer_state)

  IMP_OWN_CONSTRUCTOR(SingletonFunctor)
  IMP_OWN_CONSTRUCTOR(PairFunctor)
}

%feature("ref")   Particle "IMP::internal::ref($this);"
%feature("unref") Particle "IMP::internal::unref($this);"

%feature("ref")   Model "IMP::internal::ref($this);"
%feature("unref") Model "IMP::internal::unref($this);"

%feature("ref")   Optimizer "IMP::internal::ref($this);"
%feature("unref") Optimizer "IMP::internal::unref($this);"



/* Don't wrap internal functions */
%ignore IMP::internal::check_particles_active;

/* Make selected classes extensible in Python */
%feature("director") IMP::UnaryFunction;
%feature("director") IMP::Restraint;
%feature("director") IMP::ScoreState;
%feature("director") IMP::OptimizerState;
%feature("director") IMP::SingletonScore;
%feature("director") IMP::PairScore;
%feature("director") IMP::TripletScore;
%feature("director") IMP::Optimizer;
%feature("director") IMP::Refiner;
%feature("director") IMP::SingletonContainer;
%feature("director") IMP::PairContainer;
%feature("director") IMP::SingletonModifier;
%feature("director") IMP::PairModifier;
    

%include "IMP/base_types.h"
%include "IMP/utility.h"
%include "IMP/deprecation.h"
%include "IMP/VersionInfo.h"
%include "IMP/RefCounted.h"
%include "IMP/Object.h"
%include "IMP/UnaryFunction.h"
%include "IMP/DerivativeAccumulator.h"
%include "Restraint.i"
%include "IMP/ScoreState.h"
%include "IMP/stream.h"
%include "IMP/OptimizerState.h"
%include "IMP/log.h"
%include "IMP/Model.h"
%include "IMP/PairScore.h"
%include "IMP/Refiner.h"
%include "IMP/SingletonScore.h"
%include "IMP/TripletScore.h"
%include "Particle.i"
%include "IMP/Decorator.h"
%include "IMP/Optimizer.h"
%include "IMP/SingletonContainer.h"
%include "IMP/PairContainer.h"
%include "IMP/SingletonModifier.h"
%include "IMP/PairModifier.h"
    

namespace IMP {
  %template(Particles) ::std::vector<Particle*>;
  %template(ParticlesList) ::std::vector<Particles>;
  %template(ParticlePairs) ::std::vector<ParticlePair>;
  %template(ParticleTriplets) ::std::vector<ParticleTriplet>;
  %template(Restraints) ::std::vector<IMP::Restraint*>;
  %template(ScoreStates) ::std::vector<ScoreState*>;
  %template(OptimizerStates) ::std::vector<OptimizerState*>;
  %template(FloatKeys) ::std::vector<FloatKey>;
  %template(StringKeys) ::std::vector<StringKey>;
  %template(IntKeys) ::std::vector<IntKey>;
  %template(ParticleKeys) ::std::vector<ParticleKey>;
  %template(Floats) ::std::vector<Float>;
  %template(Strings) ::std::vector<String>;
  %template(Ints) ::std::vector<Int>;
  %template(SingletonContainers) ::std::vector<SingletonContainer*>;
  %template(PairContainers) ::std::vector<PairContainer*>;
  %template(float_pair) ::std::pair<float,float>;
  %template(double_pair) ::std::pair<double, double>;
}
