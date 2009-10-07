/**
 *  \file ClosePairsScoreState.h    \brief Maintain a list of close pairs.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_CLOSE_PAIRS_SCORE_STATE_H
#define IMPCORE_CLOSE_PAIRS_SCORE_STATE_H

#include "config.h"
#include "MaximumChangeXYZRScoreState.h"
#include "ClosePairsFinder.h"
#include "ListPairContainer.h"
#include <IMP/ScoreState.h>
#include <IMP/PairFilter.h>

#include <IMP/SingletonContainer.h>

IMPCORE_BEGIN_NAMESPACE

// for SWIG
class ListPairContainer;
class MaximumChangeXYZRScoreState;
class ClosePairsFinder;

//! Maintains a list of spatially close pairs of particles
/** An object of this class fills a ListPairContainer with
    all pairs of particles whose inter-sphere distance is
    smaller than the distance parameter.

    In order to do this efficiently the class actually computes all
    pairs within distance+slack of one another. As long as the particles
    don't move more than the slack amount, the list is still valid and
    doesn't need to be recomputed. The ClosePairsScoreState keeps track
    internally of how far the particles have moved.

    The default distance is 0 and default slack is 1.

    Here is a simple example of using this for a nonbonded list
    \verbinclude nonbonded_interactions.py

    \todo Need a mechanism to allow small updates to the set of particles.
    It is not clear how to accomplish this. Perhaps MaximumChangeScoreState
    could keep track of newly added particles. It can do that.

    \note This class uses the IMP::core::BoxSweepClosePairsFinder by
    default if \ref CGAL "CGAL" is available.

    \ingroup CGAL
    \see CloseBipartitePairsScoreState
    \see ClosePairsFinder
 */
class IMPCOREEXPORT ClosePairsScoreState : public ScoreState
{
  Pointer<MaximumChangeXYZRScoreState> xyzc_;
  Pointer<ClosePairsFinder> f_;
  Pointer<SingletonContainer> in_;
  Pointer<ListPairContainer> out_;
  Float distance_, slack_;
  FloatKey rk_;

  void initialize();
public:
  // rk needs to be there so that we don't get an error for missing attributs
  //! Create the score state for particles in the container in
  /** rk can be FloatKey() in which case the radius is ignored. */
  ClosePairsScoreState(SingletonContainer *in,
                       FloatKey rk= XYZR::get_default_radius_key());
  // rk needs to be there so that we don't get an error for missing attributs
  //! Create the score state for particles in the container in
  /** rk can be FloatKey() in which case the radius is ignored.
      The close pairs are placed in out.
  */
  ClosePairsScoreState(SingletonContainer *in,
                       ListPairContainer *out,
                       FloatKey rk= XYZR::get_default_radius_key());

  //! Set the distance threshold
  void set_distance(Float d);

  //! Set the amount of slack to add to the threshold
  /** Increasing this means the list is rebuilt more often but
      also the list is larger.
   */
  void set_slack(Float s);

  //! Get the container where the list of close pairs is put
  ListPairContainer* get_close_pairs_container() const {
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

  /** @name Methods to control the set of filters

     PairContainer objects can be used as filters to prevent
     the addition of pairs to the containeroutput list. Pairs
     which are contained in any container added to this list
     will be excluded from the close pairs list.
  */
  /**@{*/
  IMP_LIST(public, ClosePairFilter, close_pair_filter,
           PairFilter*, PairFilters);
   /**@}*/

  IMP_SCORE_STATE(ClosePairsScoreState, get_module_version_info())
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_CLOSE_PAIRS_SCORE_STATE_H */
