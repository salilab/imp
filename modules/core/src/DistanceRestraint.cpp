/**
 *  \file DistanceRestraint.cpp \brief Distance restraint between two particles.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/DistanceRestraint.h>
#include <IMP/core/DistancePairScore.h>
#include <IMP/core/XYZ.h>

#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/log.h>

IMPCORE_BEGIN_NAMESPACE

DistanceRestraint::DistanceRestraint(UnaryFunction* score_func,
                                     Particle* p1, Particle* p2) :
  TupleRestraint<DistancePairScore>(new DistancePairScore(score_func),
                                    ParticlePair(p1, p2))
{}


IMPCORE_END_NAMESPACE
