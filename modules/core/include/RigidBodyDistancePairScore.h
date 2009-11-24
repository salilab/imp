/**
 *  \file RigidBodyDistancePairScore.h
 *  \brief Act on the distance between two rigid bodies
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_RIGID_BODY_DISTANCE_PAIR_SCORE_H
#define IMPCORE_RIGID_BODY_DISTANCE_PAIR_SCORE_H

#include "config.h"
#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/PairScore.h>
#include <IMP/Refiner.h>

IMPCORE_BEGIN_NAMESPACE

//! Accelerated computation of the distance between two rigid bodies
/** Rigid bodies are made of a collection of particles whose internal
    relationships don't change. The distance between two rigid bodies
    can be defined as the minimal distance over all bipartite pairs
    with one particle taken from each rigid body. This computation can
    be accelerated (similarly to the RigidClosePairsFinder). The passed
    PairScore is then applied to this minimal pair to give the returned
    score.
 */
class IMPCOREEXPORT RigidBodyDistancePairScore: public PairScore
{
  IMP::internal::OwnerPointer<Refiner> r0_, r1_;
  IMP::internal::OwnerPointer<PairScore> ps_;
  ObjectKey k0_, k1_;
public:
  /** Use the passed refiner for both particles. */
  RigidBodyDistancePairScore(PairScore *ps, Refiner *r);
  /** Use r0 for the first passed particle and r1 for the second. */
  RigidBodyDistancePairScore(PairScore *ps, Refiner *r0, Refiner *r1);

  IMP_PAIR_SCORE(RigidBodyDistancePairScore, get_module_version_info())
};


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_RIGID_BODY_DISTANCE_PAIR_SCORE_H */
