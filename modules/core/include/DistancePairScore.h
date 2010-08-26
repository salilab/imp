/**
 *  \file DistancePairScore.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_DISTANCE_PAIR_SCORE_H
#define IMPCORE_DISTANCE_PAIR_SCORE_H

#include "core_config.h"
#include <IMP/PairScore.h>
#include <IMP/UnaryFunction.h>
#include <IMP/Pointer.h>

IMPCORE_BEGIN_NAMESPACE

//! Apply a function to the distance between two particles.
/** \see SphereDistancePairScore
    \see HarmonicDistancePairScore
*/
class IMPCOREEXPORT DistancePairScore : public PairScore
{
  IMP::internal::OwnerPointer<UnaryFunction> f_;
public:
  DistancePairScore(UnaryFunction *f);
  IMP_SIMPLE_PAIR_SCORE(DistancePairScore);
};

IMP_OBJECTS(DistancePairScore, DistancePairScores);

/** This class is significantly faster than using a DistancePairScore
    with a Harmonic.
*/
class IMPCOREEXPORT HarmonicDistancePairScore: public PairScore
{
  const double x0_, k_;
public:
  HarmonicDistancePairScore(double x0, double k):  x0_(x0), k_(k){}
  IMP_SIMPLE_PAIR_SCORE(HarmonicDistancePairScore);
};

IMP_OBJECTS(HarmonicDistancePairScore, HarmonicDistancePairScores);

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_DISTANCE_PAIR_SCORE_H */
