/**
 *  \file BipartiteNonbondedListScoreState.h
 *  \brief Allow iteration through pairs of a set of atoms.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_BIPARTITE_NONBONDED_LIST_SCORE_STATE_H
#define IMPCORE_BIPARTITE_NONBONDED_LIST_SCORE_STATE_H

#include "core_exports.h"
#include "internal/core_version_info.h"
#include "NonbondedListScoreState.h"
#include "MaxChangeScoreState.h"

IMPCORE_BEGIN_NAMESPACE

//! This class maintains a list of non-bonded pairs of spheres between two sets
/** To iterate through the list of pairs use the NonbondedListScoreState::begin,
    NonbondedListScoreState::end functions.

    The radius key can be the default key. In that case the radius is ignored.

    Changes in bost coordinates and radii are handled properly.
 */
class IMPCOREEXPORT BipartiteNonbondedListScoreState:
    public NonbondedListScoreState
{
  typedef NonbondedListScoreState P;
 public:
  //! What algorithm to use to perform the computations
  enum Algorithm
  {
    //! Check all pairs of particles to see if they are close enough
    QUADRATIC,
    //! Sweep space looking for intersecting bounding boxes.
    BBOX,
    //! Choose whatever is best that is available
    DEFAULT
  };
protected:
  Algorithm a_;
  Pointer<MaxChangeScoreState> mc0_, mc1_, mcr_;

  void process_sets(const Particles &p0,
                    const Particles &p1);

  //! \internal
  void rebuild_nbl();
  void check_nbl() const;
public:

  /** \param[in] cutoff The distance cutoff to use.
      \param[in] radius The key to use to get the radius
      \param[in] a Which algorithm to use. The default is the best.
   */
  BipartiteNonbondedListScoreState(Float cutoff, FloatKey radius,
                                   Algorithm a=DEFAULT);
  /** \param[in] cutoff The distance cutoff to use.
      \param[in] radius The key to use to get the radius
      \param[in] ps0 The first set.
      \param[in] ps1 The second set.
      \param[in] a Which algorithm to use. The default is the best.
   */
  BipartiteNonbondedListScoreState(Float cutoff,
                                   FloatKey radius,
                                   const Particles &ps0,
                                   const Particles &ps1,
                                   Algorithm a=DEFAULT);

  ~BipartiteNonbondedListScoreState();
  IMP_SCORE_STATE(internal::core_version_info);

  //! Add the particles to the first set
  void add_particles_0(const Particles &ps);
  //! Add the particles to the second set
  void add_particles_1(const Particles &ps);
  //! Remove all particles
  void clear_particles();
  //! Replace the set of particles
  void set_particles(const Particles &ps0, const Particles &ps1);

  //! If there is CGAL support, a more efficient algorithm BBOX can be used
  void set_algorithm(Algorithm a);
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_BIPARTITE_NONBONDED_LIST_SCORE_STATE_H */
