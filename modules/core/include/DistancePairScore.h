/**
 *  \file IMP/core/DistancePairScore.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_DISTANCE_PAIR_SCORE_H
#define IMPCORE_DISTANCE_PAIR_SCORE_H

#include <IMP/core/core_config.h>
#include <IMP/score_functor/UnaryFunctionEvaluate.h>
#include <IMP/score_functor/Harmonic.h>
#include <IMP/score_functor/Shift.h>
#include <IMP/score_functor/distance_pair_score_macros.h>
IMPCORE_BEGIN_NAMESPACE

//! Score a pair of particles based on the distance between their centers.
/** The pairwise distance is converted to a score using a provided
    UnaryFunction.

    Note: implicitly assumes the scored particles are decorated as XYZR.

    \see XYZR
    \see SphereDistancePairScore
    \see DistancePairScore
    \see SoftSpherePairScore
*/
IMP_FUNCTOR_DISTANCE_PAIR_SCORE(
    DistancePairScore, score_functor::UnaryFunctionEvaluate,
    (UnaryFunction *uf, std::string name = "DistancePairScore%1%"), (uf));
IMP_OBJECTS(DistancePairScore, DistancePairScores);

#ifndef IMP_DOXYGEN
typedef score_functor::Shift<score_functor::Harmonic> HarmonicDistanceScore;
#endif

//! Score distance between two particle centers using a harmonic function.
/** Score a pair of particles based on the distance between their centers,
    using a harmonic function. This is faster than using a regular
    DistancePairScore combined with a Harmonic UnaryFunction.

    Note: implicitly assumes the scored particles are decorated as XYZR.

    \see XYZR
    \see SphereDistancePairScore
    \see DistancePairScore
    \see SoftSpherePairScore

*/
IMP_FUNCTOR_DISTANCE_PAIR_SCORE(
    HarmonicDistancePairScore, HarmonicDistanceScore,
    (double x0, double k, std::string name = "HarmonicDistancePairScore%1%"),
    (x0, score_functor::Harmonic(k)));
IMP_OBJECTS(HarmonicDistancePairScore, HarmonicDistancePairScores);

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_DISTANCE_PAIR_SCORE_H */
