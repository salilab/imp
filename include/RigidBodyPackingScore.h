/**
 *  \file RigidBodyPackingScore.h
 *  \brief A Score on the crossing angle between two rigid bodies
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMP_RIGID_BODY_PACKING_SCORE_H
#define IMP_RIGID_BODY_PACKING_SCORE_H

#include <IMP/membrane/HelixDecorator.h>
#include <IMP/PairScore.h>
#include <IMP/Pointer.h>

IMPMEMBRANE_BEGIN_NAMESPACE

//! Score on the crossing angles between two helices
class IMPMEMBRANEEXPORT RigidBodyPackingScore : public PairScore
{
  Floats x0_, sigma_;
  unsigned int nsig_, ncl_;
public:
  RigidBodyPackingScore(Floats x0, Floats sigma,
                        unsigned int nsig, unsigned int ncl);
  IMP_PAIR_SCORE(RigidBodyPackingScore);
};

IMP_OBJECTS(RigidBodyPackingScore, RigidBodyPackingScores);


IMPMEMBRANE_END_NAMESPACE

#endif  /* IMP_RIGID_BODY_PACKING_SCORE_H */
