/**
 *  \file IMP/core/model_statistics.h
 *  \brief Log the restraint scores and things.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_MODEL_STATISTICS_H
#define IMPCORE_MODEL_STATISTICS_H

#include <IMP/core/core_config.h>
#include "PeriodicOptimizerState.h"
#include "periodic_optimizer_state_macros.h"
#include <IMP/Model.h>
#include <IMP/OptimizerState.h>
#include <IMP/optimizer_state_macros.h>
#include <IMP/io.h>

IMPCORE_BEGIN_NAMESPACE

/** Periodically call write the scores of the passed restraints
    to a file. The file starts with a line whose first character
    is a # with the names of the restraints separated by commas
    and then has one line per periodic update with comma
    separated scores for each restraint.
*/
class IMPCOREEXPORT WriteRestraintScoresOptimizerState:
  public PeriodicOptimizerState {
  Restraints rs_;
  base::TextOutput out_;
 public:
  WriteRestraintScoresOptimizerState(const Restraints &rs,
                                     base::TextOutput out);
 protected:
  virtual void do_update(unsigned int call_num) IMP_OVERRIDE;
  IMP_OBJECT_METHODS(WriteRestraintScoresOptimizerState);
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_MODEL_STATISTICS_H */
