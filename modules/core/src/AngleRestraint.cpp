/**
 *  \file AngleRestraint.cpp \brief Angle restraint between three particles.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
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

IMPCORE_END_NAMESPACE
