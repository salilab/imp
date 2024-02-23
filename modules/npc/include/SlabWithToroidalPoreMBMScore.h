/**
 *  \file IMP/npc/SlabWithToroidalPoreMBMScore.h
 *  \brief a go-like score for a slab with a toroidal pore
 *

 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPNPC_SLAB_WITH_TOROIDAL_PORE_MBM_SCORE_H
#define IMPNPC_SLAB_WITH_TOROIDAL_PORE_MBM_SCORE_H

#include "npc_config.h"
#include "SlabWithToroidalPore.h"
#include <IMP/Model.h>
#include <IMP/Pointer.h>
#include <IMP/check_macros.h>
#include <IMP/PairScore.h>
#include <IMP/pair_macros.h>
#include "IMP/core/XYZR.h"

IMPNPC_BEGIN_NAMESPACE

//! apply harmonic walls to the distance between a particle and the normal to toroidal membrane
/** The source code is as follows:
 * \include SlabWithToroidalPoreMBMScore.h
 * \include SlabWithToroidalPoreMBMScore.cpp
 */

// scores a go-like harmonic interaction based on normal distance to closest point
class IMPNPCEXPORT SlabWithToroidalPoreMBMScore : public PairScore {
    double x0_upper_, x0_lower_, k_;

public:
/**
 * Constructs a score over a horizontal slab with a toroidal indent.

   @param k the slab repulsive force constant in kcal/mol/A
 */
SlabWithToroidalPoreMBMScore(double x0_upper, double x0_lower, double k);
virtual double evaluate_index(Model *m,
                              const ParticleIndexPair &p,
                              DerivativeAccumulator *da) const override;

virtual ModelObjectsTemp do_get_inputs(Model *m,
                                       const ParticleIndexes &pis) const override;

IMP_PAIR_SCORE_METHODS(SlabWithToroidalPoreMBMScore);
IMP_OBJECT_METHODS(SlabWithToroidalPoreMBMScore);

};

IMP_OBJECTS(SlabWithToroidalPoreMBMScore, SlabWithToroidalPoreMBMScores);

IMPNPC_END_NAMESPACE

#endif /* IMPNPC_SLAB_WITH_TOROIDAL_PORE_MBM_SCORE_H */
