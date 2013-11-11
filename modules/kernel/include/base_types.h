/**
 *  \file IMP/kernel/base_types.h    \brief Basic types used by IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_BASE_TYPES_H
#define IMPKERNEL_BASE_TYPES_H

#include <IMP/kernel/kernel_config.h>
#include "Key.h"
#include <IMP/base/object_macros.h>
#include <IMP/base/Vector.h>
#include <IMP/base/Pointer.h>
#include <IMP/base/WeakPointer.h>
#include <IMP/base/types.h>
#include <IMP/base/Array.h>
#include <IMP/base/Index.h>

IMPKERNEL_BEGIN_NAMESPACE

/** @name Attribute Keys
    Each type of attribute has an associated type of key. The keys can
    be constructed from a string. Such construction can be expensive
    and so the resulting keys should be cached.
    @{
 */

//! The type used to identify float attributes in the Particles
typedef Key<0, true> FloatKey;
IMP_VALUES(FloatKey, FloatKeys);

//! The type used to identify int attributes in the Particles
typedef Key<1, true> IntKey;
IMP_VALUES(IntKey, IntKeys);

//! The type used to identify string attributes in the Particles
typedef Key<2, true> StringKey;
IMP_VALUES(StringKey, StringKeys);

//! The type used to identify a particle attribute in the Particles
typedef Key<3, true> ParticleIndexKey;
IMP_VALUES(ParticleIndexKey, ParticleIndexKeys);

//! The type used to identify an Object attribute
typedef Key<4, true> ObjectKey;
IMP_VALUES(ObjectKey, ObjectKeys);

//! The type used to identify a non-ref counted Object attribute
typedef Key<9, true> WeakObjectKey;
IMP_VALUES(WeakObjectKey, WeakObjectKeys);

//! The type used to identify int attributes in the Particles
typedef Key<5, true> IntsKey;
IMP_VALUES(IntsKey, IntsKeys);

//! The type used to identify a particle attribute in the Particles
typedef Key<6, true> ParticleIndexesKey;
IMP_VALUES(ParticleIndexesKey, ParticleIndexesKeys);

#ifndef IMP_DOXYGEN
// for backwards compat, no way to provide a warning on this
typedef ParticleIndexesKey ParticlesKey;
typedef ParticleIndexKey ParticleKey;
#endif

//! The type used to identify a particle attribute in the Particles
typedef Key<7, true> ObjectsKey;
IMP_VALUES(ObjectsKey, ObjectsKeys);

//! The type used to identify data stored directly in the model.
typedef Key<8, true> ModelKey;
IMP_VALUES(kernel::ModelKey, ModelKeys);

/** @} */

class Restraint;
IMP_OBJECTS(Restraint, Restraints);
class ModelObject;
IMP_OBJECTS(kernel::ModelObject, ModelObjects);
typedef base::Vector<ModelObjectsTemp> ModelObjectsTemps;

class Model;
IMP_OBJECTS(kernel::Model, Models);
class RestraintSet;
IMP_OBJECTS(RestraintSet, RestraintSets);
class ScoreState;
IMP_OBJECTS(ScoreState, ScoreStates);
class ScoringFunction;
IMP_OBJECTS(ScoringFunction, ScoringFunctions);
class Container;
IMP_OBJECTS(Container, Containers);
class Particle;
typedef base::Vector<base::Pointer<Particle> > Particles;
typedef base::Vector<base::WeakPointer<Particle> > ParticlesTemp;
typedef base::Vector<ParticlesTemp> ParticlesTemps;
class OptimizerState;
IMP_OBJECTS(OptimizerState, OptimizerStates);
class SingletonContainer;
IMP_OBJECTS(SingletonContainer, SingletonContainers);
class PairContainer;
IMP_OBJECTS(PairContainer, PairContainers);
class TripletContainer;
IMP_OBJECTS(TripletContainer, TripletContainers);
class QuadContainer;
IMP_OBJECTS(QuadContainer, QuadContainers);

class SingletonScore;
IMP_OBJECTS(SingletonScore, SingletonScores);
class PairScore;
IMP_OBJECTS(PairScore, PairScores);
class TripletScore;
IMP_OBJECTS(TripletScore, TripletScores);
class QuadScore;
IMP_OBJECTS(QuadScore, QuadScores);

class SingletonPredicate;
IMP_OBJECTS(SingletonPredicate, SingletonPredicates);
class PairPredicate;
IMP_OBJECTS(PairPredicate, PairPredicates);
class TripletPredicate;
IMP_OBJECTS(TripletPredicate, TripletPredicates);
class QuadPredicate;
IMP_OBJECTS(QuadPredicate, QuadPredicates);

class SingletonModifier;
IMP_OBJECTS(SingletonModifier, SingletonModifiers);
class PairModifier;
IMP_OBJECTS(PairModifier, PairModifiers);
class TripletModifier;
IMP_OBJECTS(TripletModifier, TripletModifiers);
class QuadModifier;
IMP_OBJECTS(QuadModifier, QuadModifiers);

/** An ordered pair of particles.*/
typedef base::Array<2, base::WeakPointer<Particle>, Particle*> ParticlePair;
IMP_VALUES(ParticlePair, ParticlePairsTemp);
/** An ordered triplet of particles.*/
typedef base::Array<3, base::WeakPointer<Particle>, Particle*> ParticleTriplet;
IMP_VALUES(ParticleTriplet, ParticleTripletsTemp);
/** An ordered quad of particles.*/
typedef base::Array<4, base::WeakPointer<Particle>, Particle*> ParticleQuad;
IMP_VALUES(ParticleQuad, ParticleQuadsTemp);

class ParticleIndexTag {};
/** A unique identifier for a particle within a Model. Use it to identifying
    particles when getting and setting attributes and constructing decorators.

    See for example, \ref model_attributes "Model attributes", Decorator, and
    Model::get_particle().
*/
typedef base::Index<ParticleIndexTag> ParticleIndex;
typedef base::Vector<ParticleIndex> ParticleIndexes;

typedef base::Array<2, ParticleIndex> ParticleIndexPair;
typedef base::Array<3, ParticleIndex> ParticleIndexTriplet;
typedef base::Array<4, ParticleIndex> ParticleIndexQuad;

IMP_VALUES(ParticleIndexPair, ParticleIndexPairs);
IMP_VALUES(ParticleIndexTriplet, ParticleIndexTriplets);
IMP_VALUES(ParticleIndexQuad, ParticleIndexQuads);

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_BASE_TYPES_H */
