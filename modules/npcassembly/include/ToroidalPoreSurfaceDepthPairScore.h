/**
 *  \file IMP/npcassembly/ToroidalPoreSurfaceDepthPairScore.h
 *  \brief a go-like score for a slab with a toroidal pore
 *

 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPNPCASSEMBLY_TOROIDAL_PORE_SURFACE_DEPTH_PAIR_SCORE_H
#define IMPNPCASSEMBLY_TOROIDAL_PORE_SURFACE_DEPTH_PAIR_SCORE_H

#include "npcassembly_config.h"
#include "SlabWithToroidalPore.h"
#include <IMP/Model.h>
#include <IMP/Pointer.h>
#include <IMP/check_macros.h>
#include <IMP/PairScore.h>
#include <IMP/pair_macros.h>
#include "IMP/core/XYZR.h"

IMPNPCASSEMBLY_BEGIN_NAMESPACE

//! apply repulsive force to the distance between a particle and the normal to toroidal membrane
/** The source code is as follows:
 * \include ToroidalPoreSurfaceDepthPairScore.h
 * \include ToroidalPoreSurfaceDepthPairScore.cpp
 */

// scores a go-like harmonic interaction based on normal distance to closest point
class IMPNPCASSEMBLYEXPORT ToroidalPoreSurfaceDepthPairScore : public PairScore {
double k_;

public:
/**
 * Constructs a score over a horizontal slab with a toroidal indent.

   @param k the slab repulsive force constant in kcal/mol/A
 */
ToroidalPoreSurfaceDepthPairScore(double k);
virtual double evaluate_index(Model *m,
                              const ParticleIndexPair &p,
                              DerivativeAccumulator *da) const override;

virtual ModelObjectsTemp do_get_inputs(Model *m,
                                       const ParticleIndexes &pis) const override;

IMP_PAIR_SCORE_METHODS(ToroidalPoreSurfaceDepthPairScore);
IMP_OBJECT_METHODS(ToroidalPoreSurfaceDepthPairScore);

};

IMP_OBJECTS(ToroidalPoreSurfaceDepthPairScore, ToroidalPoreSurfaceDepthPairScores);

IMPNPCASSEMBLY_END_NAMESPACE

#endif /* IMPNPCASSEMBLY_TOROIDAL_PORE_SURFACE_DEPTH_PAIR_SCORE_H */