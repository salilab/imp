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

/* Return derivatives from unary functions */
%include "typemaps.i"
%apply double &OUTPUT { IMP::Float& deriv };

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
  %pythonprepend Model::add_restraint %{
        args[1].thisown=0
  %}
  %pythonprepend Model::add_score_state %{
        args[1].thisown=0
  %}
  %pythonprepend Optimizer::add_optimizer_state %{
        args[1].thisown=0
  %}
  %pythonprepend RestraintSet::add_restraint %{
        args[1].thisown=0
  %}
  %pythonprepend NonbondedListScoreState::add_bonded_list %{
        args[1].thisown=0
  %}
  %pythonprepend DistanceRestraint::DistanceRestraint %{
        args[0].thisown=0
  %}
  %pythonprepend AngleRestraint::AngleRestraint %{
        args[0].thisown=0
  %}
  %pythonprepend DihedralRestraint::DihedralRestraint %{
        args[0].thisown=0
  %}
  %pythonprepend TorusRestraint::TorusRestraint %{
        args[3].thisown=0
  %}
  %pythonprepend NonbondedRestraint::NonbondedRestraint %{
        args[0].thisown=0
  %}
  %pythonprepend BondDecoratorRestraint::BondDecoratorRestraint %{
        args[0].thisown=0
  %}
  %pythonprepend SingletonListRestraint::SingletonListRestraint %{
        args[0].thisown=0
  %}
  %pythonprepend PairListRestraint::PairListRestraint %{
        args[0].thisown=0
  %}
  %pythonprepend TripletChainRestraint::TripletChainRestraint %{
        args[0].thisown=0
  %}
  %pythonprepend PairChainRestraint::PairChainRestraint %{
        args[0].thisown=0
  %}
  %pythonprepend ConnectivityRestraint::ConnectivityRestraint %{
        args[0].thisown=0
  %}
  %pythonprepend DistancePairScore::DistancePairScore %{
        args[0].thisown=0
  %}
  %pythonprepend TransformedDistancePairScore::TransformedDistancePairScore %{
        args[0].thisown=0
  %}
  %pythonprepend BondCoverPairScore::BondCoverPairScore %{
        args[0].thisown=0
  %}
  %pythonprepend SphereDistancePairScore::SphereDistancePairScore %{
        args[0].thisown=0
  %}
  %pythonprepend RefineOncePairScore::RefineOncePairScore %{
        args[0].thisown=0
        args[1].thisown=0
  %}
  %pythonprepend DistanceToSingletonScore::DistanceToSingletonScore %{
        args[0].thisown=0
  %}
  %pythonprepend AttributeSingletonScore::AttributeSingletonScore %{
        args[0].thisown=0
  %}
  %pythonprepend TunnelSingletonScore::TunnelSingletonScore %{
        args[0].thisown=0
  %}
  %pythonprepend AngleTripletScore::AngleTripletScore %{
        args[0].thisown=0
  %}
  %pythonprepend MonteCarlo::add_mover %{
        args[1].thisown=0
  %}
  %pythonprepend MonteCarlo::set_local_optimizer %{
        args[1].thisown=0
  %}
  %pythonprepend VRMLLogOptimizerState::add_particle_refiner %{
        args[1].thisown=0
  %}
  %pythonprepend TypedPairScore::set_pair_score %{
        args[1].thisown=0
  %}
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

  IMP_CONTAINER_SWIG(Model, Particle, particle);
  IMP_CONTAINER_SWIG(Model, ScoreState, score_state);
  IMP_CONTAINER_SWIG(Model, Restraint, restraint);
}

%feature("ref")   Particle "$this->ref();"
%feature("unref") Particle "$this->unref(); if (! $this->get_has_ref()) delete $this;"


/* Don't wrap internal functions */
%ignore IMP::internal::evaluate_distance_pair_score;
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

%include "IMP/Key.h"
%include "IMP/Object.h"
%include "IMP/RefCountedObject.h"
%include "IMP/Index.h"
%include "IMP/base_types.h"
%include "IMP/VersionInfo.h"
%include "IMP/UnaryFunction.h"
%include "IMP/unary_functions/Harmonic.h"
%include "IMP/unary_functions/HarmonicLowerBound.h"
%include "IMP/unary_functions/HarmonicUpperBound.h"
%include "IMP/unary_functions/OpenCubicSpline.h"
%include "IMP/unary_functions/ClosedCubicSpline.h"
%include "IMP/unary_functions/Cosine.h"
%include "IMP/unary_functions/Linear.h"
%include "IMP/unary_functions/WormLikeChain.h"
%include "IMP/DerivativeAccumulator.h"
%include "IMP/Restraint.h"
%include "IMP/ScoreState.h"
%include "IMP/OptimizerState.h"
%include "IMP/log.h"
%include "IMP/Model.h"
%include "IMP/PairScore.h"
%include "IMP/ParticleRefiner.h"
%include "IMP/SingletonScore.h"
%include "IMP/TripletScore.h"
%include "IMP/Particle.h"
%include "Vector3D.i"
%include "IMP/DecoratorBase.h"
%include "IMP/decorators/bond_decorators.h"
%include "IMP/decorators/HierarchyDecorator.h"
%include "IMP/decorators/MolecularHierarchyDecorator.h"
%include "IMP/decorators/NameDecorator.h"
%include "IMP/decorators/ResidueDecorator.h"
%include "IMP/decorators/XYZDecorator.h"
%include "IMP/decorators/AtomDecorator.h"
%include "IMP/ParticleRefiner.h"
%include "IMP/particle_refiners/BondCoverParticleRefiner.h"
%include "IMP/particle_refiners/ChildrenParticleRefiner.h"
%include "IMP/Optimizer.h"
%include "IMP/optimizers/SteepestDescent.h"
%include "IMP/optimizers/ConjugateGradients.h"
%include "IMP/optimizers/MolecularDynamics.h"
%include "IMP/optimizers/BrownianDynamics.h"
%include "IMP/optimizers/Mover.h"
%include "IMP/optimizers/MoverBase.h"
%include "IMP/optimizers/MonteCarlo.h"
%include "IMP/optimizers/movers/BallMover.h"
%include "IMP/optimizers/movers/NormalMover.h"
%include "IMP/optimizers/states/VRMLLogOptimizerState.h"
%include "IMP/optimizers/states/CMMLogOptimizerState.h"
%include "IMP/optimizers/states/VelocityScalingOptimizerState.h"
%include "IMP/pair_scores/DistancePairScore.h"
%include "IMP/pair_scores/RefineOncePairScore.h"
%include "IMP/pair_scores/SphereDistancePairScore.h"
%include "IMP/pair_scores/TypedPairScore.h"
%include "IMP/pair_scores/TransformedDistancePairScore.h"
%include "IMP/particle_refiners/BondCoverParticleRefiner.h"
%include "IMP/particle_refiners/ChildrenParticleRefiner.h"
%include "IMP/singleton_scores/DistanceToSingletonScore.h"
%include "IMP/singleton_scores/AttributeSingletonScore.h"
%include "IMP/singleton_scores/TunnelSingletonScore.h"
%include "IMP/triplet_scores/AngleTripletScore.h"
%include "IMP/score_states/BondedListScoreState.h"
%include "IMP/score_states/MaxChangeScoreState.h"
%include "IMP/score_states/NonbondedListScoreState.h"
%include "IMP/score_states/AllNonbondedListScoreState.h"
%include "IMP/score_states/BondDecoratorListScoreState.h"
%include "IMP/score_states/BipartiteNonbondedListScoreState.h"
%include "IMP/score_states/GravityCenterScoreState.h"
%include "IMP/score_states/CoverBondsScoreState.h"
%include "IMP/restraints/AngleRestraint.h"
%include "IMP/restraints/BondDecoratorRestraint.h"
%include "IMP/restraints/ConnectivityRestraint.h"
%include "IMP/restraints/ConstantRestraint.h"
%include "IMP/restraints/DihedralRestraint.h"
%include "IMP/restraints/DistanceRestraint.h"
%include "IMP/restraints/NonbondedRestraint.h"
%include "IMP/restraints/PairChainRestraint.h"
%include "IMP/restraints/PairListRestraint.h"
%include "IMP/restraints/RestraintSet.h"
%include "IMP/restraints/SingletonListRestraint.h"
%include "IMP/restraints/TripletChainRestraint.h"

namespace IMP {
  %template(ParticleIndex) Index<ParticleTag>;
  %template(RestraintIndex) Index<RestraintTag>;
  %template(ScoreStateIndex) Index<ScoreStateTag>;
  %template(OptimizerStateIndex) Index<OptimizerStateTag>;
  %template(MoverIndex) Index<Mover>;
  %template(BondedListIndex) Index<BondedListScoreState>;
  %template(FloatKey) KeyBase<Float>;
  %template(IntKey) KeyBase<Int>;
  %template(StringKey) KeyBase<String>;
  %template(ParticleKey) KeyBase<Particle*>;
  %template(AtomType) KeyBase<AtomTypeTag>;
  %template(ResidueType) KeyBase<ResidueTypeTag>;     
  %template(show_named_hierarchy) show<NameDecorator>;
  %template(show_molecular_hierarchy) show<MolecularHierarchyDecorator>;
  %template(Particles) ::std::vector<Particle*>;
  %template(ParticlePair) ::std::pair<IMP::Particle*, IMP::Particle*>;
  %template(ParticlePairs) ::std::vector<ParticlePair>;
  %template(Restraints) ::std::vector<Restraint*>;
  %template(ScoreStates) ::std::vector<ScoreState*>;
  %template(OptimizerStates) ::std::vector<OptimizerState*>;
  %template(ParticleIndexes) ::std::vector<ParticleIndex>;
  %template(BondDecorators) ::std::vector<BondDecorator>;
  %template(MolecularHiearchyDecorators) ::std::vector<MolecularHierarchyDecorator>;
  %template(FloatKeys) ::std::vector<FloatKey>;
  %template(StringKeys) ::std::vector<StringKey>;
  %template(IntKeys) ::std::vector<IntKey>;
  %template(ParticleKeys) ::std::vector<ParticleKey>;
  %template(Floats) ::std::vector<Float>;
  %template(Strings) ::std::vector<String>;
  %template(Ints) ::std::vector<Int>;
}
