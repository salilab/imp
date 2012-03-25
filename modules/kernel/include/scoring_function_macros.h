/**
 *  \file IMP/scoring_function_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_SCORING_FUNCTION_MACROS_H
#define IMPKERNEL_SCORING_FUNCTION_MACROS_H
#include "kernel_config.h"
#include <IMP/base/doxygen_macros.h>
#include <IMP/base/object_macros.h>
#include "ScoringFunction.h"


/** Declare the methods needed for an IMP::ScoringFunction object. It
    declares the following methods that you need to implement:
    - IMP::ScoringFunction::do_evaluate_if_good()
    - IMP::ScoringFunction::do_evaluate()
    - IMP::ScoringFunction::do_evaluate_if_below()
    - IMP::ScoringFunction::get_restraints()
    in addition to the IMP_OBJECT() methods.*/
#define IMP_SCORING_FUNCTION(Name)                                      \
  IMP_IMPLEMENT(ScoreIsGoodPair                                         \
                do_evaluate_if_good(bool derivatives,\
                                    const ScoreStatesTemp &ss));        \
  IMP_IMPLEMENT(ScoreIsGoodPair                                         \
                do_evaluate(bool derivatives,                           \
                            const ScoreStatesTemp &ss));                \
  IMP_IMPLEMENT(ScoreIsGoodPair                                         \
                 do_evaluate_if_below(bool derivatives,                 \
                                      double max,                       \
                                      const ScoreStatesTemp &ss));      \
  IMP_IMPLEMENT(Restraints create_restraints() const);                  \
  IMP_OBJECT(Name)
//! @}


#endif  /* IMPKERNEL_SCORING_FUNCTION_MACROS_H */
