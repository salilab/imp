%module(directors="1") IMP
%{
#include "IMP.h"
%}

%include "std_vector.i"
%include "std_map.i"
%include "std_string.i"

%include "IMP_macros.i"
%include "IMP_exceptions.i"


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
  %pythonprepend DistanceRestraint::DistanceRestraint %{
        args[3].thisown=0
  %}
  %pythonprepend SphericalRestraint::SphericalRestraint %{
        args[5].thisown=0
  %}
}

%feature("director");

%include "IMP/Object.h"
%include "IMP/Index.h"
%include "IMP/base_types.h"
%include "IMP.h"
%include "IMP/Key.h"
%include "IMP/ScoreFunc.h"
%include "IMP/ModelData.h"
%include "IMP/DerivativeAccumulator.h"
%include "IMP/Restraint.h"
%include "IMP/restraints/RestraintSet.h"
%include "IMP/State.h"
%include "IMP/log.h"
%include "IMP/Model.h"
%include "IMP/Particle.h"
%include "IMP/DecoratorBase.h"
%include "IMP/decorators/HierarchyDecorator.h"
%include "IMP/decorators/MolecularHierarchyDecorator.h"
%include "IMP/decorators/NameDecorator.h"
%include "IMP/decorators/ResidueDecorator.h"
%include "IMP/decorators/XYZDecorator.h"
%include "IMP/decorators/graph_base.h"
%include "IMP/decorators/bond_decorators.h"
%include "IMP/decorators/AtomDecorator.h"
%include "IMP/Optimizer.h"
%include "IMP/optimizers/SteepestDescent.h"
%include "IMP/optimizers/ConjugateGradients.h"
%include "IMP/optimizers/MolecularDynamics.h"
%include "IMP/restraints/DistanceRestraint.h"
%include "IMP/restraints/TorusRestraint.h"
%include "IMP/restraints/SphericalRestraint.h"
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
  %template(Particles) ::std::vector<Particle*>;       
  %template(Restraints) ::std::vector<Restraint*>;       
  %template(States) ::std::vector<State*>;       
  %template(ParticleIndexes) ::std::vector<ParticleIndex>;       
  %template(FloatKeys) ::std::vector<FloatKey>;
  %template(StringKeys) ::std::vector<StringKey>;
  %template(IntKeys) ::std::vector<IntKey>;
  %template(Floats) ::std::vector<Float>;
  %template(Strings) ::std::vector<String>;
  %template(Ints) ::std::vector<Int>;
}
