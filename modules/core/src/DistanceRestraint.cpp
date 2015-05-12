/**
 *  \file DistanceRestraint.cpp \brief Distance restraint between two particles.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
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

DistanceRestraint::DistanceRestraint(UnaryFunction* score_func,
                                     Particle* p1, Particle* p2,
                                     std::string name)
    : IMP::internal::TupleRestraint<DistancePairScore>(
          new DistancePairScore(score_func), p1->get_model(),
          ParticleIndexPair( p1->get_index(), p2->get_index() ),
          name) {
  IMPCORE_DEPRECATED_METHOD_DECL(2.5,
                                 "Use the index-based constructor instead.");
}

IMPCORE_END_NAMESPACE
