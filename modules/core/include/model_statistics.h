/**
 *  \file model_statistics.h
 *  \brief Log the restraint scores and things.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_MODEL_STATISTICS_H
#define IMPCORE_MODEL_STATISTICS_H

#include "core_config.h"
#include <IMP/Model.h>
#include <IMP/OptimizerState.h>
#include <IMP/io.h>

IMPCORE_BEGIN_NAMESPACE

/** Periodically call write the scores of the passed restraints
    to a file. The file starts with a line whose first character
    is a # with the names of the restraints separated by commas
    and then has one line per periodic update with comma
    separated scores for each restraint.
*/
class IMPCOREEXPORT WriteRestraintScoresOptimizerState:
  public OptimizerState {
  Restraints rs_;
  base::TextOutput out_;
 public:
  WriteRestraintScoresOptimizerState(const Restraints &rs,
                                     base::TextOutput out);
  IMP_PERIODIC_OPTIMIZER_STATE(WriteRestraintScoresOptimizerState);
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_MODEL_STATISTICS_H */
