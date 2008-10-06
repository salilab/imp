/**
 *  \file MaxChangeScoreState.h
 *  \brief Keep track of the maximum change of a set of attributes.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMPCORE_MAX_CHANGE_SCORE_STATE_H
#define __IMPCORE_MAX_CHANGE_SCORE_STATE_H

#include "core_exports.h"
#include "internal/core_version_info.h"

#include <IMP/ScoreState.h>
#include <IMP/Index.h>
#include <IMP/Particle.h>

#include <vector>

IMPCORE_BEGIN_NAMESPACE

//! Keeps track of the maximum change of a set of attributes.
/** The score state maintains a list of particle and a list of
    float attribute keys and keeps track of the maximum amount
    any of these have changed since the last time reset was called.

 */
class IMPCOREEXPORT MaxChangeScoreState: public ScoreState
{
  FloatKeys keys_;
  FloatKeys origkeys_;
  float max_change_;
public:
  //! Track the changes with the specified keys.
  MaxChangeScoreState(const FloatKeys &keys,
                      const Particles &ps= Particles());

  virtual ~MaxChangeScoreState(){}

  IMP_SCORE_STATE(internal::core_version_info);

  //! Measure differences from the current value.
  void reset();

  //! Return the maximum amount any attribute has changed.
  float get_max() const {
    return max_change_;
  }

  IMP_LIST(public, Particle, particle, Particle*);
};

IMPCORE_END_NAMESPACE

#endif  /* __IMPCORE_MAX_CHANGE_SCORE_STATE_H */
