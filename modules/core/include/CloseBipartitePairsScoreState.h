/**
 *  \file CloseBipartitePairsScoreState.h
 *  \brief Maintain a list of close pairs.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_CLOSE_BIPARTITE_PAIRS_SCORE_STATE_H
#define IMPCORE_CLOSE_BIPARTITE_PAIRS_SCORE_STATE_H

#include "config.h"
#include "MaximumChangeXYZRScoreState.h"
#include "ClosePairsFinder.h"
#include "ListPairContainer.h"
#include <IMP/ScoreState.h>
#include <IMP/SingletonContainer.h>


IMPCORE_BEGIN_NAMESPACE

// for SWIG
class ListPairContainer;
class MaximumChangeScoreState;
class ClosePairsFinder;

//! Maintains a list of spatially close pairs of particles between two sets
/** See ClosePairsScoreState for a general overview of the ClosePairs classes.
    The only difference from that case is that only pairs with one particle
    from each of two containers are reported here.

    \verbinclude bipartite_nonbonded_interactions.py

    \note This class uses the IMP::core::BoxSweepClosePairsFinder by
    default if \ref CGAL "CGAL" is available.

    \ingroup CGAL
    \see ClosePairsScoreState
 */
class IMPCOREEXPORT CloseBipartitePairsScoreState : public ScoreState
{
  Pointer<MaximumChangeXYZRScoreState> xyzc_[2];
  Pointer<ClosePairsFinder> f_;
  Pointer<SingletonContainer> in_[2];
  Pointer<ListPairContainer> out_;
  Float distance_, slack_;
  FloatKey rk_;

  void initialize();
  void clear();
public:
  // rk needs to be there so that we don't get an error for missing attributs
  //! Create the score state for particles in the container in
  /** rk can be FloatKey() in which case the radius is ignored. */
  CloseBipartitePairsScoreState(SingletonContainer *in0,
                                SingletonContainer *in1,
                       FloatKey rk= XYZR::get_default_radius_key());
  // rk needs to be there so that we don't get an error for missing attributs
  //! Create the score state for particles in the container in
  /** rk can be FloatKey() in which case the radius is ignored.
      The close pairs are placed in out.
  */
  CloseBipartitePairsScoreState(SingletonContainer *in0,
                                SingletonContainer *in1,
                                ListPairContainer *out,
                                FloatKey rk= XYZR::get_default_radius_key());

  //! Set the distance threshold
  void set_distance(Float d);

  //! Set the amount of slack to add to the threshold
  /** Increasing this means the list is rebuilt more often but
      also the list is larger. Unfortunately, this is hard to get right.
   */
  void set_slack(Float s);

  //! Get the container where the close pairs will be put.
  /** Add filters to this container to, for example, exclude bonds.*/
  ListPairContainer* get_close_pairs_container() const {
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

  //! Set the algorithm to use
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

  /** @name Methods to control the set of filters

     PairContainer objects can be used as filters to prevent
     the addition of pairs to the containeroutput list. Pairs
     which are contained in any container added to this list
     will be excluded from the close pairs list.
  */
  /**@{*/
  IMP_LIST(public, ClosePairFilter, close_pair_filter,
           PairContainer*, PairContainers);
  /**@}*/

  IMP_SCORE_STATE(CloseBipartitePairsScoreState, get_module_version_info())
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_CLOSE_BIPARTITE_PAIRS_SCORE_STATE_H */
