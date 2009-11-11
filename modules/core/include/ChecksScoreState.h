/**
 *  \file ChecksScoreState.h    \brief Turn checks on and off.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_CHECKS_SCORE_STATE_H
#define IMPCORE_CHECKS_SCORE_STATE_H

#include "config.h"
#include <IMP/macros.h>
#include <IMP/ScoreState.h>

IMPCORE_BEGIN_NAMESPACE

/** Turn checks on with a given probability each evaluate call.
 */
class IMPCOREEXPORT ChecksScoreState : public ScoreState
{
  double probability_;
  unsigned int count_;
 public:
  ChecksScoreState(double probability);

  unsigned int get_number_of_checked() const {
    return count_;
  }

  IMP_SCORE_STATE(ChecksScoreState, get_module_version_info())
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_CHECKS_SCORE_STATE_H */
