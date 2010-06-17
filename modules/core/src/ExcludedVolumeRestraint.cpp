/**
 *  \file ExcludedVolumeRestraint.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/core/ExcludedVolumeRestraint.h"
#include <IMP/PairContainer.h>
#include <IMP/core/XYZR.h>
#include <IMP/core/FixedRefiner.h>
#include <IMP/core/internal/CorePairsRestraint.h>
#include <IMP/core/CoverRefined.h>
#include <IMP/core/HarmonicLowerBound.h>
#include <IMP/core/RigidClosePairsFinder.h>
#include <IMP/core/SphereDistancePairScore.h>
#include <IMP/core/DistancePairScore.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/core/internal/evaluate_distance_pair_score.h>
#include <IMP/core/internal/close_pairs_helpers.h>
#include <boost/lambda/lambda.hpp>

IMPCORE_BEGIN_NAMESPACE

namespace {
  PairContainer* get_close_pairs_container(SingletonContainer *sc,
                                           Refiner *r) {
    if (r) {
      IMP_NEW(RigidClosePairsFinder, rcpf, (r));
      return new internal::CoreClosePairContainer(sc, 0.0, rcpf, 1.0);
    } else {
      return new internal::CoreClosePairContainer(sc, 0.0,
                                                  internal::default_cpf(), 1.0);
    }
  }
  PairScore* get_sphere_distance_pair_score(double k) {
    return new SphereDistancePairScore(new HarmonicLowerBound(0,k));
  }
}

ExcludedVolumeRestraint::ExcludedVolumeRestraint(SingletonContainer *sc,
                                                 Refiner *r,
                                                 double k):
  internal::CorePairsRestraint(get_sphere_distance_pair_score(k),
                               get_close_pairs_container(sc, r),
                               "ExcludedVolumeRestraint %d"){
}

ExcludedVolumeRestraint::ExcludedVolumeRestraint(SingletonContainer *sc,
                                                 double k):
  internal::CorePairsRestraint(get_sphere_distance_pair_score(k),
                               get_close_pairs_container(sc, NULL),
                               "ExcludedVolumeRestraint %d"){
}



IMPCORE_END_NAMESPACE
