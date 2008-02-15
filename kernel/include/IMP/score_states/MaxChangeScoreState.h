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
class IMPDLLEXPORT MaxChangeScoreState: public ScoreState
{
  FloatKeys keys_;
  std::vector<std::vector<float> > orig_;
  Particles ps_;
  float max_change_;
public:
  MaxChangeScoreState(const FloatKeys &keys):keys_(keys){}
  virtual ~MaxChangeScoreState(){}

  virtual void update();

  void reset();

  float get_max() const {
    return max_change_;
  }

  IMP_LIST(public, Particle, particle, Particle*);
};

} // namespace IMP

#endif  /* __IMP_MAX_CHANGE_SCORE_STATE_H */
