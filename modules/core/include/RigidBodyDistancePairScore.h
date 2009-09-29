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

/*
P\left(\mathop{\arg\min}_{p_0 \in R_0, p_1 \in R_1}
    \mathop{distance}\left(\mathop{XYZR}\left(p_0 \right),
    \mathop{XYZR}\left(p_1 \right) \right)\right)

\operatornamewithlimits{argmax}_{p_0 \in R_0, p_1 \in R_1}
    \operatorname{distance}\left( p_0, p_1\right)

\left( p_0, p_1\right)
*/

//! Accelerated computation of the distance between two rigid bodies
/** Rigid bodies are made of a collection of particles whose internal
    relationships don't change. The distance between two rigid bodies
    can be defined as the minimal distance over all bipartite pairs
    with one particle taken from each rigid body. This computation can
    be accelerated (similarly to the RigidClosePairsFinder).

    Specifically, given a PairScore, \f$P\f$ and two rigid bodies
    \f$R_0,\R_1\f$, this score computes
    \f[
    P\left(\operatorname{distance}  \right)
    \f]
 */
class IMPCOREEXPORT RigidBodyDistancePairScore: public PairScore
{
  Pointer<Refiner> r_;
  Pointer<PairScore> ps_;
  ObjectKey k_;
public:
  //! XXXX
  RigidBodyDistancePairScore(PairScore *ps, Refiner *r);

  IMP_PAIR_SCORE(RigidBodyDistancePairScore, get_module_version_info())
};


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_RIGID_BODY_DISTANCE_PAIR_SCORE_H */
