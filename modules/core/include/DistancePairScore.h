/**
 *  \file DistancePairScore.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_DISTANCE_PAIR_SCORE_H
#define IMPCORE_DISTANCE_PAIR_SCORE_H

#include "config.h"
#include "internal/version_info.h"
#include <IMP/PairScore.h>
#include <IMP/UnaryFunction.h>
#include <IMP/Pointer.h>

IMPCORE_BEGIN_NAMESPACE

//! Apply a function to the distance between two particles.
/** \see SphereDistancePairScore
*/
class IMPCOREEXPORT DistancePairScore : public PairScore
{
  Pointer<UnaryFunction> f_;
public:
  DistancePairScore(UnaryFunction *f);
  IMP_PAIR_SCORE(DistancePairScore, internal::version_info)
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_DISTANCE_PAIR_SCORE_H */
