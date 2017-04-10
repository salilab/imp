/**
 *  \file DistanceRestraint.cpp \brief Distance restraint between two particles.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/DistanceRestraint.h>
#include <IMP/core/DistancePairScore.h>
#include <IMP/core/XYZ.h>

#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/log.h>

IMPCORE_BEGIN_NAMESPACE

DistanceRestraint::DistanceRestraint(Model *m, UnaryFunction* score_func,
                                     ParticleIndexAdaptor p1,
                                     ParticleIndexAdaptor p2,
                                     std::string name)
    : IMP::internal::TupleRestraint<DistancePairScore>(
          new DistancePairScore(score_func), m,
          ParticleIndexPair(p1, p2), name) {}

IMPCORE_END_NAMESPACE
