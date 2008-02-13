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

  virtual void update() {
    max_change_=0;
    for (unsigned int i=0; i < orig_.size(); ++i) {
      for (unsigned int j=0; j < keys_.size(); ++j) {
        max_change_= std::max(max_change_,
                              std::abs(ps_[i]->get_value(keys_[j])
                                       - orig_[i][j]));
      }
    }
  }

  void reset() {
    orig_.resize(ps_.size(), std::vector<float>(keys_.size(), 0));
    for (unsigned int i=0; i < orig_.size(); ++i) {
      for (unsigned int j=0; j < keys_.size(); ++j) {
        orig_[i][j]=-ps_[i]->get_value(keys_[j]);
      }
    }
    max_change_=0;
  }

  float get_max() const {
    return max_change_;
  }

  //! Set the set of particles used
  void set_particles(const Particles &ps) {
    ps_=ps;
    reset();
  }

  const Particles &get_particles() const {
    return ps_;
  }

};

} // namespace IMP

#endif  /* __IMP_MAX_CHANGE_SCORE_STATE_H */
