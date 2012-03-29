/**
 *  \file KinkPairScore.h
 *  \brief A Score on the crossing angle between two rigid bodies
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPMEMBRANE_KINK_PAIR_SCORE_H
#define IMPMEMBRANE_KINK_PAIR_SCORE_H

#include "membrane_config.h"

#include <IMP/PairScore.h>
#include <IMP/Pointer.h>
#include <IMP/UnaryFunction.h>

IMPMEMBRANE_BEGIN_NAMESPACE

//! Score on the crossing angles between two helices
class IMPMEMBRANEEXPORT KinkPairScore : public PairScore
{
  IMP::internal::OwnerPointer<UnaryFunction> f_;
public:
  KinkPairScore(UnaryFunction *f);

  IMP_PAIR_SCORE(KinkPairScore);
};

IMP_OBJECTS(KinkPairScore, KinkPairScores);


IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_KINK_PAIR_SCORE_H */
