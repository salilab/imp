/**
 *  \file MaxChangeScoreState.h
 *  \brief Keep track of the maximum change of a set of attributes.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_MAX_CHANGE_SCORE_STATE_H
#define __IMP_MAX_CHANGE_SCORE_STATE_H

#include "../ScoreState.h"
#include "../Index.h"
#include "../Particle.h"

#include <vector>

namespace IMP
{

//! Keeps track of the maximum change of a set of attributes.
/** The score state maintains a list of particle and a list of
    float attribute keys and keeps track of the maximum amount
    any of these have changed since the last time reset was called.

 */
class IMPDLLEXPORT MaxChangeScoreState: public ScoreState
{
  FloatKeys keys_;
  FloatKeys origkeys_;
  Particles ps_;
  float max_change_;
  virtual void do_before_evaluate();
public:
  //! Track the changes with the specified keys.
  MaxChangeScoreState(const FloatKeys &keys,
                      const Particles &ps= Particles());

  virtual ~MaxChangeScoreState(){}

  //! Measure differences from the current value.
  void reset();

  //! Return the maximum amount any attribute has changed.
  float get_max() const {
    return max_change_;
  }

  IMP_LIST(public, Particle, particle, Particle*);
};

} // namespace IMP

#endif  /* __IMP_MAX_CHANGE_SCORE_STATE_H */
