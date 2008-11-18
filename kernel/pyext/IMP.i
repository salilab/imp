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
}

%feature("ref")   Particle "$this->ref();"
%feature("unref") Particle "$this->unref(); if (! $this->get_has_ref()) delete $this;"


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
%feature("director") IMP::ParticleRefiner;

%include "IMP/base_types.h"
%include "Vector3D.i"
%include "IMP/Object.h"
%include "IMP/RefCountedObject.h"
%include "IMP/Index.h"
%include "IMP/VersionInfo.h"
%include "IMP/UnaryFunction.h"
%include "IMP/DerivativeAccumulator.h"
%include "Restraint.i"
%include "IMP/ScoreState.h"
%include "IMP/OptimizerState.h"
%include "IMP/log.h"
%include "IMP/Model.h"
%include "IMP/PairScore.h"
%include "IMP/ParticleRefiner.h"
%include "IMP/SingletonScore.h"
%include "IMP/TripletScore.h"
%include "Particle.i"
%include "IMP/DecoratorBase.h"
%include "IMP/Optimizer.h"

namespace IMP {
  %template(ParticleIndex) Index<ParticleTag>;
  %template(RestraintIndex) Index<RestraintTag>;
  %template(ScoreStateIndex) Index<ScoreStateTag>;
  %template(OptimizerStateIndex) Index<OptimizerStateTag>;
  %template(Particles) ::std::vector<Particle*>;
  %template(ParticlesList) ::std::vector<Particles>;
  %template(ParticlePair) ::std::pair<IMP::Particle*, IMP::Particle*>;
  %template(ParticlePairs) ::std::vector<ParticlePair>;
  %template(Restraints) ::std::vector<IMP::Restraint*>;
  %template(ScoreStates) ::std::vector<ScoreState*>;
  %template(OptimizerStates) ::std::vector<OptimizerState*>;
  %template(ParticleIndexes) ::std::vector<ParticleIndex>;
  %template(FloatKeys) ::std::vector<FloatKey>;
  %template(StringKeys) ::std::vector<StringKey>;
  %template(IntKeys) ::std::vector<IntKey>;
  %template(ParticleKeys) ::std::vector<ParticleKey>;
  %template(Floats) ::std::vector<Float>;
  %template(Strings) ::std::vector<String>;
  %template(Ints) ::std::vector<Int>;
  /*%template(Vector3D) VectorD<3>;
  %template(random_vector_on_sphere) random_vector_on_sphere<3>;
  %template(random_vector_in_sphere) random_vector_in_sphere<3>;
  %template(random_vector_in_box) random_vector_in_box<3>;*/
}
