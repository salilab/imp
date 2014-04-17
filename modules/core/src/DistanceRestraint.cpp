/**
 *  \file DistanceRestraint.cpp \brief Distance restraint between two particles.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/DistanceRestraint.h>
#include <IMP/core/DistancePairScore.h>
#include <IMP/core/XYZ.h>

#include <IMP/kernel/Particle.h>
#include <IMP/kernel/Model.h>
#include <IMP/base/log.h>

IMPCORE_BEGIN_NAMESPACE

DistanceRestraint::DistanceRestraint(UnaryFunction* score_func,
                                     kernel::Particle* p1, kernel::Particle* p2,
                                     std::string name
)
    : IMP::internal::TupleRestraint<DistancePairScore>(
          new DistancePairScore(score_func), p1->get_model(),
          kernel::ParticleIndexPair(p1->get_index(), p2->get_index())) {}

IMPCORE_END_NAMESPACE
