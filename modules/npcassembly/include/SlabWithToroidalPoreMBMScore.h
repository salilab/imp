/**
 *  \file IMP/npcassembly/SlabWithToroidalPoreMBMScore.h
 *  \brief a go-like score for a slab with a toroidal pore
 *

 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPNPCASSEMBLY_SLAB_WITH_TOROIDAL_PORE_MBM_SCORE_H
#define IMPNPCASSEMBLY_SLAB_WITH_TOROIDAL_PORE_MBM_SCORE_H

#include "npcassembly_config.h"
#include "SlabWithToroidalPore.h"
#include <IMP/Model.h>
#include <IMP/Pointer.h>
#include <IMP/check_macros.h>
#include <IMP/PairScore.h>
#include <IMP/pair_macros.h>
#include "IMP/core/XYZR.h"

IMPNPCASSEMBLY_BEGIN_NAMESPACE

//! apply harmonic walls to the distance between a particle and the normal to toroidal membrane
/** The source code is as follows:
 * \include SlabWithToroidalPoreMBMScore.h
 * \include SlabWithToroidalPoreMBMScore.cpp
 */

// scores a go-like harmonic interaction based on normal distance to closest point
class IMPNPCASSEMBLYEXPORT SlabWithToroidalPoreMBMScore : public PairScore {
    double x0_upper_, x0_lower_, k_;

public:
/**
 * Constructs a score over a horizontal slab with a toroidal indent.

   @param k the slab repulsive force constant in kcal/mol/A
 */
SlabWithToroidalPoreMBMScore(double x0_upper, double x0_lower, double k);
virtual double evaluate_index(Model *m,
                              const ParticleIndexPair &p,
                              DerivativeAccumulator *da) const IMP_OVERRIDE;

virtual ModelObjectsTemp do_get_inputs(Model *m,
                                       const ParticleIndexes &pis) const IMP_OVERRIDE;

IMP_PAIR_SCORE_METHODS(SlabWithToroidalPoreMBMScore);
IMP_OBJECT_METHODS(SlabWithToroidalPoreMBMScore);

};

IMP_OBJECTS(SlabWithToroidalPoreMBMScore, SlabWithToroidalPoreMBMScores);

IMPNPCASSEMBLY_END_NAMESPACE

#endif /* IMPNPCASSEMBLY_SLAB_WITH_TOROIDAL_PORE_MBM_SCORE_H */
