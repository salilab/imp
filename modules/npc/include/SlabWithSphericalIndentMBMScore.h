/**
 *  \file IMP/npc/SlabWithSphericalIndentMBMScore.h
 *  \brief A Score on the distance between a particle and the surface of a spherical indent in a plane.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 */

#ifndef IMPNPC_SLAB_WITH_SPHERICAL_INDENT_MBM_SCORE_H
#define IMPNPC_SLAB_WITH_SPHERICAL_INDENT_MBM_SCORE_H

#include "npc_config.h"
#include "SlabWithSphericalIndent.h"
#include <IMP/core/XYZ.h>
#include <IMP/PairScore.h>
#include <IMP/pair_macros.h>
#include <IMP/UnaryFunction.h>
#include <IMP/Pointer.h>

IMPNPC_BEGIN_NAMESPACE

//! Apply a harmonic to the distance between two particles.
/** The source code is as follows:
    \include SlabWithSphericalIndentMBMScore.h
    \include SlabWithSphericalIndentMBMScore.cpp
*/
class IMPNPCEXPORT SlabWithSphericalIndentMBMScore : public PairScore {
double x0_, k_;

//private:
//inline double get_surface_distance(algebra::Vector3D* out_translation) const;

public:
//! Constructs a horizontal slab with a toroidal pore,
//! centered at the z=0 plane
/**
   Constructs a score over a horizontal slab with a spherical indent

   @param k the slab repulsive force constant in kcal/mol/A
*/
SlabWithSphericalIndentMBMScore(double x0, double k);
virtual double evaluate_index(Model *m,
                              const ParticleIndexPair &p,
                              DerivativeAccumulator *da) const override;

virtual ModelObjectsTemp do_get_inputs(
        Model *m, const ParticleIndexes &pis) const override;

IMP_PAIR_SCORE_METHODS(SlabWithSphericalIndentMBMScore);
IMP_OBJECT_METHODS(SlabWithSphericalIndentMBMScore);
};

IMP_OBJECTS(SlabWithSphericalIndentMBMScore, SlabWithSphericalIndentMBMScores);

IMPNPC_END_NAMESPACE

#endif /* IMPNPC_SLAB_WITH_SPHERICAL_INDENT_MBM_SCORE_H */
