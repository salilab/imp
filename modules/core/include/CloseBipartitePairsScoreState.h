/**
 *  \file CloseBipartitePairsScoreState.h
 *  \brief Maintain a list of close pairs.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_CLOSE_BIPARTITE_PAIRS_SCORE_STATE_H
#define IMPCORE_CLOSE_BIPARTITE_PAIRS_SCORE_STATE_H

#include "core_exports.h"
#include "MaximumChangeScoreState.h"
#include "ClosePairsFinder.h"
#include "SingletonContainer.h"
#include "FilteredListPairContainer.h"
#include <IMP/ScoreState.h>

IMPCORE_BEGIN_NAMESPACE

// for SWIG
class FilteredListPairContainer;
class MaximumChangeScoreState;
class ClosePairsFinder;

//! Maintains a list of spatially close pairs of particles between two sets
/** See the documentation for ClosePairsScoreState first. The only
    difference from that case is that pairs with one particle
    from each of two containers are reportedhere.
 */
class IMPCOREEXPORT CloseBipartitePairsScoreState : public ScoreState
{
  Pointer<MaximumChangeScoreState> xyzc_[2];
  Pointer<MaximumChangeScoreState> rc_[2];
  Pointer<ClosePairsFinder> f_;
  Pointer<SingletonContainer> in_[2];
  Pointer<FilteredListPairContainer> out_;
  Float distance_, slack_;
  FloatKey rk_;

  void initialize();
  void clear();
public:
  // rk needs to be there so that we don't get an error for missing attributs
  CloseBipartitePairsScoreState(SingletonContainer *in0,
                                SingletonContainer *in1,
                       FloatKey rk= XYZRDecorator::get_default_radius_key());
  // rk needs to be there so that we don't get an error for missing attributs
  CloseBipartitePairsScoreState(SingletonContainer *in0,
                                SingletonContainer *in1,
                       FilteredListPairContainer *out,
                       FloatKey rk= XYZRDecorator::get_default_radius_key());

  virtual ~CloseBipartitePairsScoreState();

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

  //! Set the first container to get particles from
  void set_first_singleton_container(SingletonContainer *pc);

  //! Set the first container to get particles from
  void set_second_singleton_container(SingletonContainer *pc);


  //! Get the container with the set of particles
  SingletonContainer* get_first_singleton_container() const {
    return in_[0];
  }

  //! Get the container with the set of particles
  SingletonContainer* get_second_singleton_container() const {
    return in_[1];
  }


  void set_close_pairs_finder(ClosePairsFinder *f);

  void set_radius_key(FloatKey rk);

  FloatKey get_radius_key() const {
    return rk_;
  }

  Float get_distance() const {
    return distance_;
  }

  IMP_SCORE_STATE(internal::core_version_info)
};

IMP_OUTPUT_OPERATOR(CloseBipartitePairsScoreState);

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_CLOSE_BIPARTITE_PAIRS_SCORE_STATE_H */
