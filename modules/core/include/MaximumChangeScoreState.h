/**
 *  \file MaximumChangeScoreState.h
 *  \brief Keep track of the Maximumimum change of a set of attributes.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_MAXIMUM_CHANGE_SCORE_STATE_H
#define IMPCORE_MAXIMUM_CHANGE_SCORE_STATE_H

#include "config.h"
#include "internal/core_version_info.h"

#include <IMP/ScoreState.h>
#include <IMP/Particle.h>
#include <IMP/SingletonContainer.h>

#include <vector>

IMPCORE_BEGIN_NAMESPACE

//! Keeps track of the maximum change of a set of attributes.
/** The score state maintains a list of particle and a list of
    float attribute keys and keeps track of the Maximumimum amount
    any of these have changed since the last time reset was called.

    Adding a new particle to the container without calling reset()
    results in a max change of std::numeric_limits<Float>::max().

    \ingroup restraint
 */
class IMPCOREEXPORT MaximumChangeScoreState: public ScoreState
{
  typedef IMP::internal::
    AttributeTable<IMP::internal::FloatAttributeTableTraits> AT;
  FloatKeys keys_;
  std::map<Particle*, AT> orig_values_;
  float maximum_change_;
  Pointer<SingletonContainer> pc_;
public:
  //! Track the changes with the specified keys.
  MaximumChangeScoreState(SingletonContainer *pc,
                          const FloatKeys &keys);

  virtual ~MaximumChangeScoreState();

  IMP_SCORE_STATE(internal::core_version_info);

  //! Measure differences from the current value.
  void reset();

  //! Return the maximum amount any attribute has changed.
  float get_maximum_change() const {
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

#endif  /* IMPCORE_MAXIMUM_CHANGE_SCORE_STATE_H */
