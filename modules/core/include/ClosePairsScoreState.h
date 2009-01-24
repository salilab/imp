/**
 *  \file ClosePairsScoreState.h    \brief Maintain a list of close pairs.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_CLOSE_PAIRS_SCORE_STATE_H
#define IMPCORE_CLOSE_PAIRS_SCORE_STATE_H

#include "config.h"
#include "MaximumChangeScoreState.h"
#include "ClosePairsFinder.h"
#include "FilteredListPairContainer.h"
#include <IMP/ScoreState.h>

#include <IMP/SingletonContainer.h>

IMPCORE_BEGIN_NAMESPACE

// for SWIG
class FilteredListPairContainer;
class MaximumChangeScoreState;
class ClosePairsFinder;

//! Maintains a list of spatially close pairs of particles
/** On object of this class fills a PairContainer with
    spatially close pairs of particles. More precisely, a
    ClosePairsFinder is used to fill an output list of type
    FilteredListPairContainer from an input set of particles
    store in a SingletonContainer.

    In order to do this efficiently, it increases the distance threshold
    by some amount of slack. As long as the particles don't move more than
    the slack amount, the list is still valid and doesn't need to be
    recomputed.

    The default distance is 0 and default slack is 1.

    Here is a simple example of using this for a nonbonded list
    \verbinclude nonbonded_interactions.py

    \todo Need a mechanism to allow small updates to the set of particles.
    It is not clear how to accomplish this. Perhaps MaximumChangeScoreState
    could keep track of newly added particles. It can do that.

    \ingroup restraint
 */
class IMPCOREEXPORT ClosePairsScoreState : public ScoreState
{
  Pointer<MaximumChangeScoreState> xyzc_;
  Pointer<MaximumChangeScoreState> rc_;
  Pointer<ClosePairsFinder> f_;
  Pointer<SingletonContainer> in_;
  Pointer<FilteredListPairContainer> out_;
  Float distance_, slack_;
  FloatKey rk_;

  void initialize();
public:
  // rk needs to be there so that we don't get an error for missing attributs
  ClosePairsScoreState(SingletonContainer *in,
                       FloatKey rk= XYZRDecorator::get_default_radius_key());
  // rk needs to be there so that we don't get an error for missing attributs
  ClosePairsScoreState(SingletonContainer *in,
                       FilteredListPairContainer *out,
                       FloatKey rk= XYZRDecorator::get_default_radius_key());

  virtual ~ClosePairsScoreState();

  //! Set the distance threshold
  void set_distance(Float d);

  //! Set the amount of slack to add to the threshold
  /** Increasing this means the list is rebuilt more often but
      also the list is larger. Unfortunately, this is hard to get right.
   */
  void set_slack(Float s);

  FilteredListPairContainer* get_close_pairs_container() const {
    return out_;
  }

  //! Set the container to get particles from
  void set_singleton_container(SingletonContainer *pc);

  //! Get the container with the set of particles
  SingletonContainer* get_singleton_container() const {
    return in_;
  }

  //! Set the algorithm used.
  void set_close_pairs_finder(ClosePairsFinder *f);

  //! Set the key used to find the radius.
  /** It can be FloatKey() in which case all radii are 0.*/
  void set_radius_key(FloatKey rk);

  //! Get the key used to find the radius.
  FloatKey get_radius_key() const {
    return rk_;
  }

  //! Get the distance threshold.
  Float get_distance() const {
    return distance_;
  }

  IMP_SCORE_STATE(internal::core_version_info)
};

IMP_OUTPUT_OPERATOR(ClosePairsScoreState);

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_CLOSE_PAIRS_SCORE_STATE_H */
