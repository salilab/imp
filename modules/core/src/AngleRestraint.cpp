/**
 *  \file AngleRestraint.cpp \brief Angle restraint between three particles.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/AngleRestraint.h>
#include <IMP/core/AngleTripletScore.h>

IMPCORE_BEGIN_NAMESPACE

AngleRestraint::AngleRestraint(Model *m, UnaryFunction* score_func,
                               ParticleIndexAdaptor p1, ParticleIndexAdaptor p2,
                               ParticleIndexAdaptor p3)
    : TripletRestraint(m, new AngleTripletScore(score_func),
                       ParticleIndexTriplet(p1, p2, p3)) {}

AngleRestraint::AngleRestraint(UnaryFunction* score_func, Particle* p1,
                               Particle* p2, Particle* p3)
    : TripletRestraint(new AngleTripletScore(score_func),
                       ParticleTriplet(p1, p2, p3)) {
  IMPCORE_DEPRECATED_METHOD_DECL(2.5,
                                 "Use the index-based constructor instead.");
}

AngleRestraint::AngleRestraint(UnaryFunction* score_func, XYZ p0, XYZ p1,
                               XYZ p2)
    : TripletRestraint(new AngleTripletScore(score_func),
                       ParticleTriplet(p0, p1, p2)) {
  IMPCORE_DEPRECATED_METHOD_DECL(2.5,
                                 "Use the index-based constructor instead.");
}

IMPCORE_END_NAMESPACE
