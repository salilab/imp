/**
 *  \file IMP/core/DistancePairScore.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_DISTANCE_PAIR_SCORE_H
#define IMPCORE_DISTANCE_PAIR_SCORE_H

#include <IMP/core/core_config.h>
#include <IMP/score_functor/UnaryFunctionEvaluate.h>
#include <IMP/score_functor/Harmonic.h>
#include <IMP/score_functor/Shift.h>
#include <IMP/score_functor/distance_pair_score_macros.h>
IMPCORE_BEGIN_NAMESPACE

/** Score a pair of particles based on their distance using an
    arbitrary UnaryFunction.*/
IMP_FUNCTOR_DISTANCE_PAIR_SCORE(
    DistancePairScore, score_functor::UnaryFunctionEvaluate,
    (UnaryFunction *uf, std::string name = "DistancePairScore%1%"), (uf));

#ifndef IMP_DOXYGEN
typedef score_functor::Shift<score_functor::Harmonic> HarmonicDistanceScore;
#endif

/** Score a pair of particles based on their distance using a
    Harmonic. This is faster than DistancePairScore if you
    are using a Harmonic.*/
IMP_FUNCTOR_DISTANCE_PAIR_SCORE(
    HarmonicDistancePairScore, HarmonicDistanceScore,
    (double x0, double k, std::string name = "HarmonicDistancePairScore%1%"),
    (x0, score_functor::Harmonic(k)));
IMPCORE_END_NAMESPACE

#endif /* IMPCORE_DISTANCE_PAIR_SCORE_H */
