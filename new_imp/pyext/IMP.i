%module(directors="1") IMP
%{
#include "IMP.h"
%}

/* Ignore shared object import/export stuff */
#define IMPDLLEXPORT
#define IMPDLLLOCAL
#define IMP_COMPARISONS_1(f) bool operator==(const This &o) const ; bool operator<(const This &o) const; bool operator>(const This &o) const; bool operator!=(const This &o) const; bool operator<=(const This &o) const; bool operator>=(const This &o) const;
#define IMP_OUTPUT_OPERATOR_1(f)
#define IMP_OUTPUT_OPERATOR(f)
#define IMP_RESTRAINT(a,b) \
  virtual Float evaluate(DerivativeAccumulator *accum);\
  virtual void show(std::ostream &out) const;\
  virtual std::string version() const {return std::string(version_string);}\
  virtual std::string last_modified_by() const {return std::string(lmb_string);}
#define IMP_DECORATOR_GET(a,b,c,d)
#define IMP_DECORATOR_GET_SET(n,a,t,r) r get_##n() const;\
                                       void set_##n(t);
#define IMP_DECORATOR_GET_SET_OPT(n,a,t,r,d) r get_##n() const;\
                                       void set_##n(t);
#define IMP_DECORATOR(Name, Parent, a,b) \
        public:\
        typedef Name This;\
                public: Name(); \
                static Name create(Particle *p);\
                static Name cast(Particle *p);\
                void show(std::ostream &=std::cout, std::string pre="")
#define IMP_DECORATOR_ARRAY_DECL(a, b) 


%include "std_vector.i"
%include "std_map.i"
%include "std_string.i"

%include "IMP_exceptions.i"

namespace std {
  %template(vectori) vector<int>;
  %template(vectorf) vector<float>;
}

namespace IMP {
  %pythonprepend Model::add_particle %{
        args[1].thisown=0
  %}
  %pythonprepend Model::add_restraint %{
        args[1].thisown=0
  %}
  %pythonprepend Model::add_state %{
        args[1].thisown=0
  %}
  %pythonprepend RestraintSet::add_restraint %{
        args[1].thisown=0
  %}
}

%feature("director");

%include "IMP/Index.h"
%include "IMP/Base_Types.h"
%include "IMP.h"
%include "IMP/boost/noncopyable.h"
%include "IMP/Key.h"
%include "IMP/ScoreFunc.h"
%include "IMP/ModelData.h"
%include "IMP/DerivativeAccumulator.h"
%include "IMP/restraints/Restraint.h"
%include "IMP/restraints/RestraintSet.h"
%include "IMP/State.h"
%include "IMP/log.h"
%include "IMP/Model.h"
%include "IMP/Particle.h"
%include "IMP/decorators/DecoratorBase.h"
%include "IMP/decorators/HierarchyDecorator.h"
%include "IMP/decorators/MolecularHierarchyDecorator.h"
%include "IMP/decorators/NameDecorator.h"
%include "IMP/decorators/ResidueDecorator.h"
%include "IMP/decorators/XYZDecorator.h"
%include "IMP/decorators/graph_base.h"
%include "IMP/decorators/bond_decorators.h"
%include "IMP/decorators/AtomDecorator.h"
%include "IMP/optimizers/Optimizer.h"
%include "IMP/optimizers/SteepestDescent.h"
%include "IMP/optimizers/ConjugateGradients.h"
%include "IMP/optimizers/MolecularDynamics.h"
%include "IMP/restraints/DistanceRestraint.h"
%include "IMP/restraints/TorusRestraint.h"
%include "IMP/restraints/CoordinateRestraint.h"
%include "IMP/restraints/ProximityRestraint.h"
%include "IMP/restraints/ConnectivityRestraint.h"
%include "IMP/restraints/PairConnectivityRestraint.h"
%include "IMP/restraints/ExclusionVolumeRestraint.h"
%include "IMP/RigidBody.h"

namespace IMP {
  %template(IntIndex) Index<Int>;
  %template(FloatIndex) Index<Float>;
  %template(StringIndex) Index<String>;
  %template(ParticleIndex) Index<ParticleTag>;
  %template(RestraintIndex) Index<RestraintTag>;
  %template(StateIndex) Index<StateTag>;
  %template(FloatKey) Key<Float>;
  %template(IntKey) Key<Int>;
  %template(StringKey) Key<String>;
  %template(AtomType) Key<AtomTypeTag>;
  %template(ResidueType) Key<ResidueTypeTag>;     
  %template(show_named_nierarchy) show<NameDecorator>;
  %template(show_molecular_hierarchy) show<MolecularHierarchyDecorator>;
  %template(ParticleVector) ::std::vector<Particle*>;       
}
