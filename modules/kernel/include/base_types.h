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
#include "key_macros.h"
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
IMP_DECLARE_KEY_TYPE(FloatKey, 0);
//! The type used to identify int attributes in the Particles
IMP_DECLARE_KEY_TYPE(IntKey, 1);
//! The type used to identify string attributes in the Particles
IMP_DECLARE_KEY_TYPE(StringKey, 2);
//! The type used to identify a particle attribute in the Particles
IMP_DECLARE_KEY_TYPE(ParticleIndexKey, 3);
//! The type used to identify an Object attribute
IMP_DECLARE_KEY_TYPE(ObjectKey, 4);
//! The type used to identify a non-ref counted Object attribute
IMP_DECLARE_KEY_TYPE(WeakObjectKey, 9);
//! The type used to identify int attributes in the Particles
IMP_DECLARE_KEY_TYPE(IntsKey, 5);
//! The type used to identify a particle attribute in the Particles
IMP_DECLARE_KEY_TYPE(ParticleIndexesKey, 6);

#ifndef IMP_DOXYGEN
// for backwards compat, no way to provide a warning on this
typedef ParticleIndexesKey ParticlesKey;
typedef ParticleIndexKey ParticleKey;
#endif

//! The type used to identify a particle attribute in the Particles
IMP_DECLARE_KEY_TYPE(ObjectsKey, 7);

//! The type used to identify data stored directly in the model.
IMP_DECLARE_KEY_TYPE(ModelKey, 8);

/** @} */


class Restraint;
IMP_OBJECTS(Restraint,Restraints);
class ModelObject;
IMP_OBJECTS(ModelObject,ModelObjects);
typedef base::Vector<ModelObjectsTemp> ModelObjectsTemps;

class Model;
IMP_OBJECTS(Model,Models);
class RestraintSet;
IMP_OBJECTS(RestraintSet,RestraintSets);
class ScoreState;
IMP_OBJECTS(ScoreState,ScoreStates);
class ScoringFunction;
IMP_OBJECTS(ScoringFunction,ScoringFunctions);
class Container;
IMP_OBJECTS(Container,Containers);
class Particle;
typedef base::Vector<base::Pointer<Particle> > Particles;
typedef base::Vector<base::WeakPointer<Particle> > ParticlesTemp;
typedef base::Vector<ParticlesTemp > ParticlesTemps;
class OptimizerState;
IMP_OBJECTS(OptimizerState,OptimizerStates);
class SingletonContainer;
IMP_OBJECTS(SingletonContainer,SingletonContainers);
class PairContainer;
IMP_OBJECTS(PairContainer,PairContainers);
class TripletContainer;
IMP_OBJECTS(TripletContainer,TripletContainers);
class QuadContainer;
IMP_OBJECTS(QuadContainer,QuadContainers);

class SingletonScore;
IMP_OBJECTS(SingletonScore,SingletonScores);
class PairScore;
IMP_OBJECTS(PairScore,PairScores);
class TripletScore;
IMP_OBJECTS(TripletScore,TripletScores);
class QuadScore;
IMP_OBJECTS(QuadScore,QuadScores);

class SingletonPredicate;
IMP_OBJECTS(SingletonPredicate,SingletonPredicates);
class PairPredicate;
IMP_OBJECTS(PairPredicate,PairPredicates);
class TripletPredicate;
IMP_OBJECTS(TripletPredicate,TripletPredicates);
class QuadPredicate;
IMP_OBJECTS(QuadPredicate,QuadPredicates);

class SingletonModifier;
IMP_OBJECTS(SingletonModifier,SingletonModifiers);
class PairModifier;
IMP_OBJECTS(PairModifier,PairModifiers);
class TripletModifier;
IMP_OBJECTS(TripletModifier,TripletModifiers);
class QuadModifier;
IMP_OBJECTS(QuadModifier,QuadModifiers);



/** An ordered pair of particles.*/
typedef base::Array<2, base::WeakPointer<Particle>, Particle*>
ParticlePair;
IMP_VALUES(ParticlePair, ParticlePairsTemp);
/** An ordered triplet of particles.*/
typedef base::Array<3, base::WeakPointer<Particle>, Particle*>
ParticleTriplet;
IMP_VALUES(ParticleTriplet, ParticleTripletsTemp);
/** An ordered quad of particles.*/
typedef base::Array<4, base::WeakPointer<Particle>, Particle*>
ParticleQuad;
IMP_VALUES(ParticleQuad, ParticleQuadsTemp);


class ParticleIndexTag{};
/** A unique identifier for a particle within a Model. Use it to get
    attributes and do other operations.*/
typedef base::Index<ParticleIndexTag> ParticleIndex;
typedef base::Vector<ParticleIndex> ParticleIndexes;

typedef base::Array<2, ParticleIndex> ParticleIndexPair;
typedef base::Array<3, ParticleIndex> ParticleIndexTriplet;
typedef base::Array<4, ParticleIndex> ParticleIndexQuad;

IMP_VALUES(ParticleIndexPair, ParticleIndexPairs);
IMP_VALUES(ParticleIndexTriplet, ParticleIndexTriplets);
IMP_VALUES(ParticleIndexQuad, ParticleIndexQuads);

IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_BASE_TYPES_H */
