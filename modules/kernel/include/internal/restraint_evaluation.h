/**
 *  \file internal/utility.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_RESTRAINT_EVALUATION_H
#define IMPKERNEL_INTERNAL_RESTRAINT_EVALUATION_H

#include <IMP/kernel_config.h>
#include "../base_types.h"
#include "../ScoreAccumulator.h"

IMP_BEGIN_INTERNAL_NAMESPACE

IMPEXPORT void protected_evaluate(IMP::ScoreAccumulator sa,
                                  Restraint* restraint,
                                  const ScoreStatesTemp &states,
                                  Model *m);


IMPEXPORT void protected_evaluate(IMP::ScoreAccumulator sa,
                         const RestraintsTemp &restraints,
                                               const ScoreStatesTemp &states,
                         Model *m);
IMPEXPORT void protected_evaluate(IMP::ScoreAccumulator sa,
                         const Restraints &restraints,
                                              const ScoreStatesTemp &states,
                         Model *m);

IMP_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_RESTRAINT_EVALUATION_H */
