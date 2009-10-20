/**
 *  \file MaximumChangeXYZRScoreState.h
 *  \brief Keep track of the maximum change of a set of attributes.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_MAXIMUM_CHANGE_XYZR_SCORE_STATE_H
#define IMPCORE_MAXIMUM_CHANGE_XYZR_SCORE_STATE_H

#include "config.h"

#include <IMP/ScoreState.h>
#include <IMP/Particle.h>
#include <IMP/SingletonContainer.h>
#include <IMP/algebra/Sphere3D.h>
#include <vector>

IMPCORE_BEGIN_NAMESPACE

//! A version of the MaximumChangeScoreState for x,y,z
/** This score state keeps track of the maximum change in extents of
    a set of particles. That is, how much there corners of a box
    centered around the cartesian coordinates with a radius of the
    particle radius, have moved.

    See MaximumChangeScoreState for a more general purpose version.*/
class IMPCOREEXPORT MaximumChangeXYZRScoreState: public ScoreState
{
  std::vector<algebra::Sphere3D> orig_values_;
  Float maximum_change_;
  IMP::internal::OwnerPointer<SingletonContainer> pc_;
  unsigned int rev_;
  FloatKey rk_;
public:
  //! Track the changes with the specified keys.
  MaximumChangeXYZRScoreState(SingletonContainer *pc,
                              FloatKey rk);

  IMP_SCORE_STATE(MaximumChangeXYZRScoreState, get_module_version_info());

  //! Measure differences from the current value.
  void reset();

  //! Return the maximum amount any attribute has changed.
  Float get_maximum_change() const {
    return maximum_change_;
  }
  //! Change the container to get the particles from
  void set_singleton_container(SingletonContainer *pc) {
    pc_=pc;
    reset();
  }
  //! Return the container storing the particles
  SingletonContainer *get_singleton_container() const {
    return pc_;
  }
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_MAXIMUM_CHANGE_XYZR_SCORE_STATE_H */
