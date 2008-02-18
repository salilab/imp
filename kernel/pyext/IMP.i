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
        args[2].thisown=0
  %}
  %pythonprepend AngleRestraint::AngleRestraint %{
        args[3].thisown=0
  %}
  %pythonprepend DihedralRestraint::DihedralRestraint %{
        args[4].thisown=0
  %}
  %pythonprepend TorusRestraint::TorusRestraint %{
        args[3].thisown=0
  %}
  %pythonprepend NonbondedRestraint::NonbondedRestraint %{
        args[1].thisown=0
  %}
  %pythonprepend BondDecoratorRestraint::BondDecoratorRestraint %{
        args[1].thisown=0
  %}
  %pythonprepend ListRestraint::ListRestraint %{
        args[1].thisown=0
  %}
  %pythonprepend ChainTripletRestraint::ChainTripletRestraint %{
        args[0].thisown=0
  %}
  %pythonprepend ConnectivityRestraint::ConnectivityRestraint %{
        args[0].thisown=0
  %}
  %pythonprepend DistancePairScore::DistancePairScore %{
        args[1].thisown=0
  %}
  %pythonprepend SphereDistancePairScore::SphereDistancePairScore %{
        args[0].thisown=0
  %}
  %pythonprepend DistanceToSingletonScore::DistanceToSingletonScore %{
        args[1].thisown=0
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
}

/* Don't wrap internal functions */
%ignore IMP::internal::evaluate_distance_pair_score;

%feature("director");

%include "IMP/Object.h"
%include "IMP/Index.h"
%include "IMP/base_types.h"
%include "IMP.h"
%include "IMP/Key.h"
%include "IMP/UnaryFunction.h"
%include "IMP/unary_functions/Harmonic.h"
%include "IMP/unary_functions/HarmonicLowerBound.h"
%include "IMP/unary_functions/HarmonicUpperBound.h"
%include "IMP/unary_functions/OpenCubicSpline.h"
%include "IMP/unary_functions/Cosine.h"
%include "IMP/unary_functions/Linear.h"
%include "IMP/ScoreFuncParams.h"
%include "IMP/DerivativeAccumulator.h"
%include "IMP/Restraint.h"
%include "IMP/ScoreState.h"
%include "IMP/OptimizerState.h"
%include "IMP/log.h"
%include "IMP/Model.h"
%include "IMP/PairScore.h"
%include "IMP/SingletonScore.h"
%include "IMP/TripletScore.h"
%include "IMP/Particle.h"
%include "IMP/Vector3D.h"
%include "IMP/DecoratorBase.h"
%include "IMP/decorators/HierarchyDecorator.h"
%include "IMP/decorators/MolecularHierarchyDecorator.h"
%include "IMP/decorators/NameDecorator.h"
%include "IMP/decorators/ResidueDecorator.h"
%include "IMP/decorators/XYZDecorator.h"
%include "IMP/decorators/bond_decorators.h"
%include "IMP/decorators/AtomDecorator.h"
%include "IMP/Optimizer.h"
%include "IMP/optimizers/SteepestDescent.h"
%include "IMP/optimizers/ConjugateGradients.h"
%include "IMP/optimizers/MolecularDynamics.h"
%include "IMP/optimizers/Mover.h"
%include "IMP/optimizers/MoverBase.h"
%include "IMP/optimizers/MonteCarlo.h"
%include "IMP/optimizers/movers/BallMover.h"
%include "IMP/optimizers/movers/NormalMover.h"
%include "IMP/optimizers/states/VRMLLogOptimizerState.h"
%include "IMP/optimizers/states/CMMLogOptimizerState.h"
%include "IMP/pair_scores/DistancePairScore.h"
%include "IMP/pair_scores/SphereDistancePairScore.h"
%include "IMP/singleton_scores/DistanceToSingletonScore.h"
%include "IMP/triplet_scores/AngleTripletScore.h"
%include "IMP/restraints/DistanceRestraint.h"
%include "IMP/restraints/AngleRestraint.h"
%include "IMP/restraints/ChainTripletRestraint.h"
%include "IMP/restraints/DihedralRestraint.h"
%include "IMP/restraints/TorusRestraint.h"
%include "IMP/restraints/ProximityRestraint.h"
%include "IMP/restraints/ConnectivityRestraint.h"
%include "IMP/restraints/PairConnectivityRestraint.h"
%include "IMP/restraints/ExclusionVolumeRestraint.h"
%include "IMP/restraints/NonbondedRestraint.h"
%include "IMP/restraints/BondDecoratorRestraint.h"
%include "IMP/restraints/ListRestraint.h"
%include "IMP/restraints/RestraintSet.h"
%include "IMP/score_states/BondedListScoreState.h"
%include "IMP/score_states/MaxChangeScoreState.h"
%include "IMP/score_states/NonbondedListScoreState.h"
%include "IMP/score_states/AllNonbondedListScoreState.h"
%include "IMP/score_states/BipartiteNonbondedListScoreState.h"
%include "IMP/score_states/BondDecoratorListScoreState.h"

namespace IMP {
  %template(IntIndex) Index<Int>;
  %template(FloatIndex) Index<Float>;
  %template(StringIndex) Index<String>;
  %template(ParticleIndex) Index<ParticleTag>;
  %template(RestraintIndex) Index<RestraintTag>;
  %template(ScoreStateIndex) Index<ScoreStateTag>;
  %template(OptimizerStateIndex) Index<OptimizerStateTag>;
  %template(MoverIndex) Index<Mover>;
  %template(BondeListIndex) Index<BondedListScoreState>;
  %template(FloatKey) Key<Float>;
  %template(IntKey) Key<Int>;
  %template(StringKey) Key<String>;
  %template(AtomType) Key<AtomTypeTag>;
  %template(ResidueType) Key<ResidueTypeTag>;     
  %template(show_named_hierarchy) show<NameDecorator>;
  %template(show_molecular_hierarchy) show<MolecularHierarchyDecorator>;
  %template(Particles) ::std::vector<Particle*>;       
  %template(Restraints) ::std::vector<Restraint*>;       
  %template(ScoreStates) ::std::vector<ScoreState*>;       
  %template(OptimizerStates) ::std::vector<OptimizerState*>;       
  %template(ParticleIndexes) ::std::vector<ParticleIndex>;       
  %template(FloatKeys) ::std::vector<FloatKey>;
  %template(StringKeys) ::std::vector<StringKey>;
  %template(IntKeys) ::std::vector<IntKey>;
  %template(Floats) ::std::vector<Float>;
  %template(Strings) ::std::vector<String>;
  %template(Ints) ::std::vector<Int>;
}
