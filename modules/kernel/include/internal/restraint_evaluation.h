/**
 *  \file internal/restraint_evaluation.h
 *  \brief Utility functions to get restraint scores
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_RESTRAINT_EVALUATION_H
#define IMPKERNEL_INTERNAL_RESTRAINT_EVALUATION_H

#include <IMP/kernel_config.h>
#include "../base_types.h"
#include "../ScoreAccumulator.h"

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

IMPKERNELEXPORT void protected_evaluate(IMP::ScoreAccumulator sa,
                                        Restraint *restraint,
                                        const ScoreStatesTemp &states,
                                        Model *m);

IMPKERNELEXPORT void protected_evaluate(IMP::ScoreAccumulator sa,
                                        const RestraintsTemp &restraints,
                                        const ScoreStatesTemp &states,
                                        Model *m);
IMPKERNELEXPORT void protected_evaluate(IMP::ScoreAccumulator sa,
                                        const Restraints &restraints,
                                        const ScoreStatesTemp &states,
                                        Model *m);

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_INTERNAL_RESTRAINT_EVALUATION_H */
