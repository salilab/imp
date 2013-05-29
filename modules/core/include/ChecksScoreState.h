/**
 *  \file IMP/core/ChecksScoreState.h    \brief Turn checks on and off.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_CHECKS_SCORE_STATE_H
#define IMPCORE_CHECKS_SCORE_STATE_H

#include <IMP/core/core_config.h>
#include <IMP/macros.h>
#include <IMP/Particle.h>
#include <IMP/ScoreState.h>

IMPCORE_BEGIN_NAMESPACE

/** Turn checks on with a given probability each evaluate call.
 */
class IMPCOREEXPORT ChecksScoreState : public ScoreState {
  double probability_;
  unsigned int num_checked_;

 public:
  ChecksScoreState(double probability);

  unsigned int get_number_of_checked() const { return num_checked_; }

  IMP_SCORE_STATE(ChecksScoreState);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_CHECKS_SCORE_STATE_H */
