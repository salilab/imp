/**
 *  \file IMP/core/SurfaceDistancePairScore.h
 *  \brief A score on the distance between a surface and a sphere.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_SURFACE_DISTANCE_PAIR_SCORE_H
#define IMPCORE_SURFACE_DISTANCE_PAIR_SCORE_H

#include <IMP/core/core_config.h>
#include "Surface.h"
#include "XYZR.h"
#include <IMP/score_functor/UnaryFunctionEvaluate.h>
#include <IMP/score_functor/PointToSphereDistance.h>
#include <IMP/score_functor/Shift.h>
#include <IMP/score_functor/Harmonic.h>
#include <IMP/score_functor/HarmonicLowerBound.h>
#include <IMP/score_functor/HarmonicUpperBound.h>
#include <IMP/score_functor/surface_distance_pair_score_macros.h>

IMPCORE_BEGIN_NAMESPACE

typedef score_functor::PointToSphereDistance<score_functor::UnaryFunctionEvaluate>
    PointToSphereDistanceScore;

//! A score on the distance between a surface and a sphere.
/** \see Surface
    \see XYZR
    \see DistancePairScore
 */
IMP_FUNCTOR_SURFACE_DISTANCE_PAIR_SCORE(SurfaceDistancePairScore,
                                SurfaceDistancePairScore,
                                PointToSphereDistanceScore,
                                (UnaryFunction *uf,
                                 std::string name =
                                     "SurfaceDistancePairScore%1%"),
                                (score_functor::UnaryFunctionEvaluate(uf)));

//! A score on the height of a sphere above a surface.
/** \see Surface
    \see XYZR
    \see SurfaceDistancePairScore
    \see DistancePairScore
 */
IMP_FUNCTOR_SURFACE_DISTANCE_PAIR_SCORE(SurfaceHeightPairScore,
                                SurfaceHeightPairScore,
                                PointToSphereDistanceScore,
                                (UnaryFunction *uf,
                                 std::string name =
                                     "SurfaceHeightPairScore%1%"),
                                (score_functor::UnaryFunctionEvaluate(uf)));

//! A score on the depth of a sphere below a surface.
/** \see Surface
    \see XYZR
    \see SurfaceDistancePairScore
    \see DistancePairScore
 */
IMP_FUNCTOR_SURFACE_DISTANCE_PAIR_SCORE(SurfaceDepthPairScore,
                                SurfaceDepthPairScore,
                                PointToSphereDistanceScore,
                                (UnaryFunction *uf,
                                 std::string name =
                                     "SurfaceDepthPairScore%1%"),
                                (score_functor::UnaryFunctionEvaluate(uf)));


typedef score_functor::PointToSphereDistance<
    score_functor::Shift<score_functor::Harmonic> > HarmonicSurfaceDistanceScore;

//! A harmonic score on the distance between a surface and a sphere.
/** \see Surface
    \see XYZR
    \see Harmonic
    \see SurfaceDistancePairScore
    \see DistancePairScore
 */
IMP_FUNCTOR_SURFACE_DISTANCE_PAIR_SCORE(HarmonicSurfaceDistancePairScore,
                                SurfaceDistancePairScore,
                                HarmonicSurfaceDistanceScore,
                                (double x0, double k,
                                 std::string name =
                                     "HarmonicSurfaceDistancePairScore%1%"),
                                (score_functor::Shift<score_functor::Harmonic>(
                                    x0, score_functor::Harmonic(k))));

// //! A harmonic score on the height of a sphere above a surface.
// /** \see Surface
//     \see XYZR
//     \see Harmonic
//     \see SurfaceHeightPairScore
//     \see DistancePairScore
//  */
IMP_FUNCTOR_SURFACE_DISTANCE_PAIR_SCORE(HarmonicSurfaceHeightPairScore,
                                SurfaceHeightPairScore,
                                HarmonicSurfaceDistanceScore,
                                (double x0, double k,
                                 std::string name =
                                     "HarmonicSurfaceHeightPairScore%1%"),
                                (score_functor::Shift<score_functor::Harmonic>(
                                    x0, score_functor::Harmonic(k))));

// //! A harmonic score on the depth of a sphere below a surface.
// /** \see Surface
//     \see XYZR
//     \see Harmonic
//     \see SurfaceDepthPairScore
//     \see DistancePairScore
//  */
IMP_FUNCTOR_SURFACE_DISTANCE_PAIR_SCORE(HarmonicSurfaceDepthPairScore,
                                SurfaceDepthPairScore,
                                HarmonicSurfaceDistanceScore,
                                (double x0, double k,
                                 std::string name =
                                     "HarmonicSurfaceDepthPairScore%1%"),
                                (score_functor::Shift<score_functor::Harmonic>(
                                    x0, score_functor::Harmonic(k))));

typedef score_functor::PointToSphereDistance<score_functor::HarmonicLowerBound>
    SoftSurfaceDistanceScore;


//! A harmonic score that keeps a sphere above a surface.
/** This class is equivalent to, but faster than a
    SurfaceHeightPairScore with a HarmonicLowerBound.

    \see HarmonicLowerBound
    \see SurfaceHeightPairScore
    \see SoftSpherePairScore
*/
IMP_FUNCTOR_SURFACE_DISTANCE_PAIR_SCORE(SoftSuperSurfacePairScore,
                          SurfaceHeightPairScore,
                          SoftSurfaceDistanceScore,
                          (double k,
                           std::string name = "SoftSuperSurfacePairScore%1%"),
                          (score_functor::HarmonicLowerBound(k)));

//! A harmonic score that keeps a sphere below a surface.
/** This class is equivalent to, but faster than a
    SurfaceDepthPairScore with a HarmonicLowerBound.

    \see HarmonicLowerBound
    \see SurfaceDepthPairScore
    \see SoftSpherePairScore
*/
IMP_FUNCTOR_SURFACE_DISTANCE_PAIR_SCORE(SoftSubSurfacePairScore,
                          SurfaceDepthPairScore,
                          SoftSurfaceDistanceScore,
                          (double k,
                           std::string name = "SoftSubSurfacePairScore%1%"),
                          (score_functor::HarmonicLowerBound(k)));


IMPCORE_END_NAMESPACE

#endif /* IMPCORE_SURFACE_DISTANCE_PAIR_SCORE_H */
