/**
 *  \file AllNonbondedListScoreState.h
 *  \brief Allow iteration through pairs of a set of s.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_ALL_NONBONDED_LIST_SCORE_STATE_H
#define __IMP_ALL_NONBONDED_LIST_SCORE_STATE_H

#include "NonbondedListScoreState.h"
#include "MaxChangeScoreState.h"
#include "../internal/kernel_version_info.h"
#include "../internal/Vector.h"


namespace IMP
{

namespace internal
{
class ParticleGrid;
}

//! This class maintains a list of non-bonded pairs of spheres
/** To iterate through the list of pairs use the NonbondedListScoreState::begin,
    NonbondedListScoreState::end functions.

    The radius key can be the default key.

    Changes in coordinates and radii are handled properly unless grid is used,
    then changes in radii may not be handled properly.

    \ingroup restraint
 */
class IMPDLLEXPORT AllNonbondedListScoreState: public NonbondedListScoreState
{
  typedef NonbondedListScoreState P;
public:
  //! What algorithm to use to perform the computations
  enum Algorithm
  {
    //! Check all pairs of particles to see if they are close enough
    QUADRATIC,
    GRID,
    //! Sweep space looking for intersecting bounding boxes.
    BBOX,
    //! Choose the best algorithm available
    DEFAULT
  };
protected:
  Algorithm a_;
  Pointer<MaxChangeScoreState> mc_, mcr_;

  void check_nbl() const;

  //! \internal
  void rebuild_nbl();


  // methods for grid
  void grid_rebuild_nbl();

  void grid_partition_points(internal::Vector<internal::ParticleGrid*> &bins);

  void grid_generate_nbl(const internal::ParticleGrid *particle_bin,
                         const internal::ParticleGrid *grid_bin);

  float grid_side_from_r(float r) const;
public:


  /** \param[in] cutoff The distance cutoff to use.
      \param[in] radius The key to use to get the radius
      \param[in] a Which algorithm to use. The default is the best.
   */
  AllNonbondedListScoreState(Float cutoff,
                             FloatKey radius,
                             Algorithm a= DEFAULT);
  /** \param[in] cutoff The distance cutoff to use.
      \param[in] radius The key to use to get the radius
      \param[in] ps A list of particles to use.
      \param[in] a Which algorithm to use. The default is the best.
   */
  AllNonbondedListScoreState(Float cutoff,
                             FloatKey radius,
                             const Particles &ps,
                             Algorithm a= DEFAULT);
  ~AllNonbondedListScoreState();

  IMP_SCORE_STATE(internal::kernel_version_info);

  //! Add the particles and add them to the NBL
  void add_particles(const Particles &ps);
  //! Remove all particles
  void clear_particles();
  //! Replace the set of particles
  void set_particles(const Particles &ps);

  //! If there is CGAL support, a more efficient algorithm BBOX can be used
  void set_algorithm(Algorithm a);
};

} // namespace IMP

#endif  /* __IMP_ALL_NONBONDED_LIST_SCORE_STATE_H */
