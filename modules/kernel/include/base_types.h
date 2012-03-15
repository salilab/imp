/**
 *  \file IMP/base_types.h    \brief Basic types used by IMP.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_BASE_TYPES_H
#define IMPKERNEL_BASE_TYPES_H

#include "kernel_config.h"
#include "Key.h"
#include "macros.h"
#include <IMP/base/ConvertibleVector.h>
#include <IMP/base/types.h>

IMP_BEGIN_NAMESPACE


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
IMP_DECLARE_KEY_TYPE(ParticleKey, 3);
//! The type used to identify a particle attribute in the Particles
IMP_DECLARE_KEY_TYPE(ObjectKey, 4);
//! The type used to identify int attributes in the Particles
IMP_DECLARE_KEY_TYPE(IntsKey, 5);
//! The type used to identify a particle attribute in the Particles
IMP_DECLARE_KEY_TYPE(ParticlesKey, 6);
//! The type used to identify a particle attribute in the Particles
IMP_DECLARE_KEY_TYPE(ObjectsKey, 7);

//! The type used to identify data stored directly in the model.
IMP_DECLARE_KEY_TYPE(ModelKey, 8);

/** @} */


class Restraint;
IMP_OBJECTS(Restraint,Restraints);
class Model;
IMP_OBJECTS(Model,Models);
class RestraintSet;
IMP_OBJECTS(RestraintSet,RestraintSets);
class ScoreState;
IMP_OBJECTS(ScoreState,ScoreStates);
class ScoringFunction;
IMP_OBJECTS(ScoringFunction,ScoringFunctions);
class Container;
IMP_OBJECTS_TYPEDEF(Container,Containers);
class Particle;
typedef base::ConvertibleVector<base::Pointer<Particle> > Particles;
typedef vector<base::WeakPointer<Particle> > ParticlesTemp;
typedef vector<vector<base::WeakPointer<Particle> > > ParticlesTemps;
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

IMP_END_NAMESPACE

#endif  /* IMPKERNEL_BASE_TYPES_H */
