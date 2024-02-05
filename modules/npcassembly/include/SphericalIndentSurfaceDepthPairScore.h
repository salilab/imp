/**
 *  \file IMP/npcassembly/SphericalIndentSurfaceDepthPairScore.h
 *  \brief A Score on the distance between a particle and the surface of a spherical indent in a plane.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 */

#ifndef IMPNPCASSEMBLY_SPHERICAL_INDENT_SURFACE_DEPTH_PAIR_SCORE_H
#define IMPNPCASSEMBLY_SPHERICAL_INDENT_SURFACE_DEPTH_PAIR_SCORE_H

#include "npcassembly_config.h"
#include "SlabWithSphericalIndent.h"
#include <IMP/core/XYZ.h>
#include <IMP/PairScore.h>
#include <IMP/pair_macros.h>
#include <IMP/UnaryFunction.h>
#include <IMP/Pointer.h>

IMPNPCASSEMBLY_BEGIN_NAMESPACE

//! Apply a harmonic to the distance between two particles.
/** The source code is as follows:
    \include SphericalIndentSurfaceDepthPairScore.h
    \include SphericalIndentSurfaceDepthPairScore.cpp
*/
class IMPNPCASSEMBLYEXPORT SphericalIndentSurfaceDepthPairScore : public PairScore {
double k_;

//private:
//inline double get_surface_distance(algebra::Vector3D* out_translation) const;

public:
//! Constructs a horizontal slab with a toroidal pore,
//! centered at the z=0 plane
/**
   Constructs a score over a horizontal slab with a spherical indent

   @param k the slab repulsive force constant in kcal/mol/A
*/
SphericalIndentSurfaceDepthPairScore(double k);
virtual double evaluate_index(Model *m,
                              const ParticleIndexPair &p,
                              DerivativeAccumulator *da) const IMP_OVERRIDE;

virtual ModelObjectsTemp do_get_inputs(
        Model *m, const ParticleIndexes &pis) const IMP_OVERRIDE;

IMP_PAIR_SCORE_METHODS(SphericalIndentSurfaceDepthPairScore);
IMP_OBJECT_METHODS(SphericalIndentSurfaceDepthPairScore);
};

IMP_OBJECTS(SphericalIndentSurfaceDepthPairScore, SphericalIndentSurfaceDepthPairScores);

IMPNPCASSEMBLY_END_NAMESPACE

#endif /* IMPNPCASSEMBLY_SPHERICAL_INDENT_SURFACE_DEPTH_PAIR_SCORE_H */
