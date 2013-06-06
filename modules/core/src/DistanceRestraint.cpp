/**
 *  \file DistanceRestraint.cpp \brief Distance restraint between two particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/DistanceRestraint.h>
#include <IMP/core/DistancePairScore.h>
#include <IMP/core/XYZ.h>

#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/base/log.h>

IMPCORE_BEGIN_NAMESPACE

DistanceRestraint::DistanceRestraint(UnaryFunction* score_func, Particle* p1,
                                     Particle* p2)
    : IMP::internal::TupleRestraint<DistancePairScore>(
          new DistancePairScore(score_func), p1->get_model(),
          ParticleIndexPair(p1->get_index(), p2->get_index())) {}

IMPCORE_END_NAMESPACE
