/**
 *  \file IMP/kernel/doxygen.h
 *  \brief Import kernel classes into the IMP scope in docs.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_DOXYGEN_H
#define IMPKERNEL_DOXYGEN_H

   #include <IMP/kernel.h>

#ifdef IMP_DOXYGEN
namespace IMP {

/** Import IMP::kernel::CommandDispatcher
    in the IMP namespace.*/
typedef IMP::kernel::CommandDispatcher
    CommandDispatcher;
/** Import IMP::kernel::Configuration
    in the IMP namespace.*/
typedef IMP::kernel::Configuration
    Configuration;
/** Import IMP::kernel::ConfigurationSet
    in the IMP namespace.*/
typedef IMP::kernel::ConfigurationSet
    ConfigurationSet;
/** Import IMP::kernel::Constraint
    in the IMP namespace.*/
typedef IMP::kernel::Constraint
    Constraint;
/** Import IMP::kernel::Container
    in the IMP namespace.*/
typedef IMP::kernel::Container
    Container;
/** Import IMP::kernel::Decorator
    in the IMP namespace.*/
typedef IMP::kernel::Decorator
    Decorator;
/** Import IMP::kernel::DependencyGraph
    in the IMP namespace.*/
typedef IMP::kernel::DependencyGraph
    DependencyGraph;
/** Import IMP::kernel::DerivativeAccumulator
    in the IMP namespace.*/
typedef IMP::kernel::DerivativeAccumulator
    DerivativeAccumulator;
/** Import IMP::kernel::EvaluationState
    in the IMP namespace.*/
typedef IMP::kernel::EvaluationState
    EvaluationState;
/** Import IMP::kernel::EvaluationStates
    in the IMP namespace.*/
typedef IMP::kernel::EvaluationStates
    EvaluationStates;
/** Import IMP::kernel::FloatIndex
    in the IMP namespace.*/
typedef IMP::kernel::FloatIndex
    FloatIndex;
/** Import IMP::kernel::FloatIndexes
    in the IMP namespace.*/
typedef IMP::kernel::FloatIndexes
    FloatIndexes;
/** Import IMP::kernel::FloatKey
    in the IMP namespace.*/
typedef IMP::kernel::FloatKey
    FloatKey;
/** Import IMP::kernel::FloatKeys
    in the IMP namespace.*/
typedef IMP::kernel::FloatKeys
    FloatKeys;
/** Import IMP::kernel::IntKey
    in the IMP namespace.*/
typedef IMP::kernel::IntKey
    IntKey;
/** Import IMP::kernel::IntKeys
    in the IMP namespace.*/
typedef IMP::kernel::IntKeys
    IntKeys;
/** Import IMP::kernel::Model
    in the IMP namespace.*/
typedef IMP::kernel::Model
    Model;
/** Import IMP::kernel::ModelObject
    in the IMP namespace.*/
typedef IMP::kernel::ModelObject
    ModelObject;
/** Import IMP::kernel::ObjectKey
    in the IMP namespace.*/
typedef IMP::kernel::ObjectKey
    ObjectKey;
/** Import IMP::kernel::ObjectKeys
    in the IMP namespace.*/
typedef IMP::kernel::ObjectKeys
    ObjectKeys;
/** Import IMP::kernel::Optimizer
    in the IMP namespace.*/
typedef IMP::kernel::Optimizer
    Optimizer;
/** Import IMP::kernel::OptimizerState
    in the IMP namespace.*/
typedef IMP::kernel::OptimizerState
    OptimizerState;
/** Import IMP::kernel::OptionParser
    in the IMP namespace.*/
typedef IMP::kernel::OptionParser
    OptionParser;
/** Import IMP::kernel::PairContainer
    in the IMP namespace.*/
typedef IMP::kernel::PairContainer
    PairContainer;
/** Import IMP::kernel::PairModifier
    in the IMP namespace.*/
typedef IMP::kernel::PairModifier
    PairModifier;
/** Import IMP::kernel::PairPredicate
    in the IMP namespace.*/
typedef IMP::kernel::PairPredicate
    PairPredicate;
/** Import IMP::kernel::PairScore
    in the IMP namespace.*/
typedef IMP::kernel::PairScore
    PairScore;
/** Import IMP::kernel::Particle
    in the IMP namespace.*/
typedef IMP::kernel::Particle
    Particle;
/** Import IMP::kernel::ParticleIndex
    in the IMP namespace.*/
typedef IMP::kernel::ParticleIndex
    ParticleIndex;
/** Import IMP::kernel::ParticleIndexKey
    in the IMP namespace.*/
typedef IMP::kernel::ParticleIndexKey
    ParticleIndexKey;
/** Import IMP::kernel::ParticleIndexKeys
    in the IMP namespace.*/
typedef IMP::kernel::ParticleIndexKeys
    ParticleIndexKeys;
/** Import IMP::kernel::ParticleIndexPairs
    in the IMP namespace.*/
typedef IMP::kernel::ParticleIndexPairs
    ParticleIndexPairs;
/** Import IMP::kernel::ParticleIndexQuads
    in the IMP namespace.*/
typedef IMP::kernel::ParticleIndexQuads
    ParticleIndexQuads;
/** Import IMP::kernel::ParticleIndexTriplets
    in the IMP namespace.*/
typedef IMP::kernel::ParticleIndexTriplets
    ParticleIndexTriplets;
/** Import IMP::kernel::ParticleIndexes
    in the IMP namespace.*/
typedef IMP::kernel::ParticleIndexes
    ParticleIndexes;
/** Import IMP::kernel::ParticleIndexesKey
    in the IMP namespace.*/
typedef IMP::kernel::ParticleIndexesKey
    ParticleIndexesKey;
/** Import IMP::kernel::ParticleIndexesKeys
    in the IMP namespace.*/
typedef IMP::kernel::ParticleIndexesKeys
    ParticleIndexesKeys;
/** Import IMP::kernel::ParticlePair
    in the IMP namespace.*/
typedef IMP::kernel::ParticlePair
    ParticlePair;
/** Import IMP::kernel::ParticlePairsTemp
    in the IMP namespace.*/
typedef IMP::kernel::ParticlePairsTemp
    ParticlePairsTemp;
/** Import IMP::kernel::ParticleQuad
    in the IMP namespace.*/
typedef IMP::kernel::ParticleQuad
    ParticleQuad;
/** Import IMP::kernel::ParticleQuadsTemp
    in the IMP namespace.*/
typedef IMP::kernel::ParticleQuadsTemp
    ParticleQuadsTemp;
/** Import IMP::kernel::ParticleTriplet
    in the IMP namespace.*/
typedef IMP::kernel::ParticleTriplet
    ParticleTriplet;
/** Import IMP::kernel::ParticleTripletsTemp
    in the IMP namespace.*/
typedef IMP::kernel::ParticleTripletsTemp
    ParticleTripletsTemp;
/** Import IMP::kernel::PythonRestraint
    in the IMP namespace.*/
typedef IMP::kernel::PythonRestraint
    PythonRestraint;
/** Import IMP::kernel::PythonScoreState
    in the IMP namespace.*/
typedef IMP::kernel::PythonScoreState
    PythonScoreState;
/** Import IMP::kernel::QuadContainer
    in the IMP namespace.*/
typedef IMP::kernel::QuadContainer
    QuadContainer;
/** Import IMP::kernel::QuadModifier
    in the IMP namespace.*/
typedef IMP::kernel::QuadModifier
    QuadModifier;
/** Import IMP::kernel::QuadPredicate
    in the IMP namespace.*/
typedef IMP::kernel::QuadPredicate
    QuadPredicate;
/** Import IMP::kernel::QuadScore
    in the IMP namespace.*/
typedef IMP::kernel::QuadScore
    QuadScore;
/** Import IMP::kernel::Refiner
    in the IMP namespace.*/
typedef IMP::kernel::Refiner
    Refiner;
/** Import IMP::kernel::Restraint
    in the IMP namespace.*/
typedef IMP::kernel::Restraint
    Restraint;
/** Import IMP::kernel::RestraintBase
    in the IMP namespace.*/
typedef IMP::kernel::RestraintBase
    RestraintBase;
/** Import IMP::kernel::RestraintSet
    in the IMP namespace.*/
typedef IMP::kernel::RestraintSet
    RestraintSet;
/** Import IMP::kernel::RestraintStatistics
    in the IMP namespace.*/
typedef IMP::kernel::RestraintStatistics
    RestraintStatistics;
/** Import IMP::kernel::RestraintStatisticsList
    in the IMP namespace.*/
typedef IMP::kernel::RestraintStatisticsList
    RestraintStatisticsList;
/** Import IMP::kernel::Sampler
    in the IMP namespace.*/
typedef IMP::kernel::Sampler
    Sampler;
/** Import IMP::kernel::SaveToConfigurationSetOptimizerState
    in the IMP namespace.*/
typedef IMP::kernel::SaveToConfigurationSetOptimizerState
    SaveToConfigurationSetOptimizerState;
/** Import IMP::kernel::ScopedRemoveRestraint
    in the IMP namespace.*/
typedef IMP::kernel::ScopedRemoveRestraint
    ScopedRemoveRestraint;
/** Import IMP::kernel::ScopedRemoveScoreState
    in the IMP namespace.*/
typedef IMP::kernel::ScopedRemoveScoreState
    ScopedRemoveScoreState;
/** Import IMP::kernel::ScopedRestraint
    in the IMP namespace.*/
typedef IMP::kernel::ScopedRestraint
    ScopedRestraint;
/** Import IMP::kernel::ScopedScoreState
    in the IMP namespace.*/
typedef IMP::kernel::ScopedScoreState
    ScopedScoreState;
/** Import IMP::kernel::ScopedSetFloatAttribute
    in the IMP namespace.*/
typedef IMP::kernel::ScopedSetFloatAttribute
    ScopedSetFloatAttribute;
/** Import IMP::kernel::ScoreAccumulator
    in the IMP namespace.*/
typedef IMP::kernel::ScoreAccumulator
    ScoreAccumulator;
/** Import IMP::kernel::ScoreAccumulators
    in the IMP namespace.*/
typedef IMP::kernel::ScoreAccumulators
    ScoreAccumulators;
/** Import IMP::kernel::ScoreState
    in the IMP namespace.*/
typedef IMP::kernel::ScoreState
    ScoreState;
/** Import IMP::kernel::ScoreStateBase
    in the IMP namespace.*/
typedef IMP::kernel::ScoreStateBase
    ScoreStateBase;
/** Import IMP::kernel::ScoringFunction
    in the IMP namespace.*/
typedef IMP::kernel::ScoringFunction
    ScoringFunction;
/** Import IMP::kernel::SingletonContainer
    in the IMP namespace.*/
typedef IMP::kernel::SingletonContainer
    SingletonContainer;
/** Import IMP::kernel::SingletonModifier
    in the IMP namespace.*/
typedef IMP::kernel::SingletonModifier
    SingletonModifier;
/** Import IMP::kernel::SingletonPredicate
    in the IMP namespace.*/
typedef IMP::kernel::SingletonPredicate
    SingletonPredicate;
/** Import IMP::kernel::SingletonScore
    in the IMP namespace.*/
typedef IMP::kernel::SingletonScore
    SingletonScore;
/** Import IMP::kernel::StringKey
    in the IMP namespace.*/
typedef IMP::kernel::StringKey
    StringKey;
/** Import IMP::kernel::StringKeys
    in the IMP namespace.*/
typedef IMP::kernel::StringKeys
    StringKeys;
/** Import IMP::kernel::TripletContainer
    in the IMP namespace.*/
typedef IMP::kernel::TripletContainer
    TripletContainer;
/** Import IMP::kernel::TripletModifier
    in the IMP namespace.*/
typedef IMP::kernel::TripletModifier
    TripletModifier;
/** Import IMP::kernel::TripletPredicate
    in the IMP namespace.*/
typedef IMP::kernel::TripletPredicate
    TripletPredicate;
/** Import IMP::kernel::TripletScore
    in the IMP namespace.*/
typedef IMP::kernel::TripletScore
    TripletScore;
/** Import IMP::kernel::UnaryFunction
    in the IMP namespace.*/
typedef IMP::kernel::UnaryFunction
    UnaryFunction;
/** Import IMP::kernel::build
    in the IMP namespace.*/
using IMP::kernel::build;
/** Import IMP::kernel::check_particle
    in the IMP namespace.*/
using IMP::kernel::check_particle;
/** Import IMP::kernel::create_decomposition
    in the IMP namespace.*/
using IMP::kernel::create_decomposition;
/** Import IMP::kernel::get_dependency_graph
    in the IMP namespace.*/
using IMP::kernel::get_dependency_graph;
/** Import IMP::kernel::get_dependent_particles
    in the IMP namespace.*/
using IMP::kernel::get_dependent_particles;
/** Import IMP::kernel::get_dependent_restraints
    in the IMP namespace.*/
using IMP::kernel::get_dependent_restraints;
/** Import IMP::kernel::get_dependent_score_states
    in the IMP namespace.*/
using IMP::kernel::get_dependent_score_states;
/** Import IMP::kernel::get_indexes
    in the IMP namespace.*/
using IMP::kernel::get_indexes;
/** Import IMP::kernel::get_input_containers
    in the IMP namespace.*/
using IMP::kernel::get_input_containers;
/** Import IMP::kernel::get_input_particles
    in the IMP namespace.*/
using IMP::kernel::get_input_particles;
/** Import IMP::kernel::get_output_containers
    in the IMP namespace.*/
using IMP::kernel::get_output_containers;
/** Import IMP::kernel::get_output_particles
    in the IMP namespace.*/
using IMP::kernel::get_output_particles;
/** Import IMP::kernel::get_particle
    in the IMP namespace.*/
using IMP::kernel::get_particle;
/** Import IMP::kernel::get_particles
    in the IMP namespace.*/
using IMP::kernel::get_particles;
/** Import IMP::kernel::get_pruned_dependency_graph
    in the IMP namespace.*/
using IMP::kernel::get_pruned_dependency_graph;
/** Import IMP::kernel::get_required_particles
    in the IMP namespace.*/
using IMP::kernel::get_required_particles;
/** Import IMP::kernel::get_required_score_states
    in the IMP namespace.*/
using IMP::kernel::get_required_score_states;
/** Import IMP::kernel::get_restraints
    in the IMP namespace.*/
using IMP::kernel::get_restraints;
/** Import IMP::kernel::get_update_order
    in the IMP namespace.*/
using IMP::kernel::get_update_order;
/** Import IMP::kernel::get_vertex_index
    in the IMP namespace.*/
using IMP::kernel::get_vertex_index;
/** Import IMP::kernel::has_boost_filesystem
    in the IMP namespace.*/
using IMP::kernel::has_boost_filesystem;
/** Import IMP::kernel::read_particles_from_buffer
    in the IMP namespace.*/
using IMP::kernel::read_particles_from_buffer;
/** Import IMP::kernel::set_score_state_update_order
    in the IMP namespace.*/
using IMP::kernel::set_score_state_update_order;
/** Import IMP::kernel::show_restraint_hierarchy
    in the IMP namespace.*/
using IMP::kernel::show_restraint_hierarchy;
/** Import IMP::kernel::write_particles_to_buffer
    in the IMP namespace.*/
using IMP::kernel::write_particles_to_buffer;
} // namespace

/** \namespace IMP

    \brief All IMP::kernel code is brought into the IMP namespace.

    All \imp code is in the \imp namespace. For convenience and backwards
    compatibility, the contents of IMP::kernel are hoisted into the \imp
    namespace itself, in addition to being in the IMP::kernel namespace.
    For now you should use the IMP namespace versions of the names.
*/

#endif //doxygen
#endif  /* IMPKERNEL_DOXYGEN_H */
