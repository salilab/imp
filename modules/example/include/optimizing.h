/**
 *  \file IMP/example/optimizing.h
 *  \brief A simple unary function.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEXAMPLE_OPTIMIZING_H
#define IMPEXAMPLE_OPTIMIZING_H

#include <IMP/example/example_config.h>
#include <IMP/log.h>
#include <IMP/base_types.h>
#include <IMP/enums.h>
#include <IMP/core/MonteCarloMover.h>
#include <IMP/enums.h>

IMPEXAMPLE_BEGIN_NAMESPACE
/** Create a serial mover from a list of core::XYZR particles.
 */
IMPEXAMPLEEXPORT core::MonteCarloMover *create_serial_mover(Model *m,
    const ParticleIndexes &pis);

/** Take a set of core::XYZR particles and relax them relative to a set of
    restraints. Excluded volume is handle separately, so don't include it
    in the passed list of restraints.

    \include optimize_balls.cpp
*/
IMPEXAMPLEEXPORT void optimize_balls(
    Model *m,
    const ParticleIndexes &pis,
    const RestraintsTemp &rs = RestraintsTemp(),
    const PairPredicates &excluded = PairPredicates(),
    const OptimizerStates &opt_states = OptimizerStates(),
    LogLevel ll = DEFAULT);

IMPEXAMPLE_END_NAMESPACE

#endif /* IMPEXAMPLE_OPTIMIZING_H */
