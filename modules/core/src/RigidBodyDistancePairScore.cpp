/**
 *  \file RigidBodyDistancePairScore.cpp
 *  \brief Act on the distance between two rigid bodies.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/core/RigidBodyDistancePairScore.h"
#include "IMP/core/internal/rigid_body_tree.h"

IMPCORE_BEGIN_NAMESPACE

RigidBodyDistancePairScore::RigidBodyDistancePairScore(PairScore *ps,
                                                       Refiner *r)
    : KClosePairsPairScore(ps, r, 1) {}

IMPCORE_END_NAMESPACE
