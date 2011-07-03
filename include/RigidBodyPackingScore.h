/**
 *  \file RigidBodyPackingScore.h
 *  \brief A Score on the crossing angle between two rigid bodies
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPMEMBRANE_RIGID_BODY_PACKING_SCORE_H
#define IMPMEMBRANE_RIGID_BODY_PACKING_SCORE_H

#include "membrane_config.h"

#include <IMP/PairScore.h>
#include <IMP/Pointer.h>
#include <IMP/core/TableRefiner.h>

IMPMEMBRANE_BEGIN_NAMESPACE

//! Score on the crossing angles between two helices
class IMPMEMBRANEEXPORT RigidBodyPackingScore : public PairScore
{
  internal::OwnerPointer<core::TableRefiner> tbr_;
  Floats omb_, ome_, ddb_, dde_;
  double kappa_;
public:
  RigidBodyPackingScore(core::TableRefiner *tbr,
                        Floats omb, Floats ome, Floats ddb,
                        Floats dde, double kappa);
  IMP_PAIR_SCORE(RigidBodyPackingScore);
};

IMP_OBJECTS(RigidBodyPackingScore, RigidBodyPackingScores);


IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_RIGID_BODY_PACKING_SCORE_H */
