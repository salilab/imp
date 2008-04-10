/**
 *  \file AllNonbondedListScoreState.h
 *  \brief Allow iteration through pairs of a set of s.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_ALL_NONBONDED_LIST_SCORE_STATE_H
#define __IMP_ALL_NONBONDED_LIST_SCORE_STATE_H

#include "NonbondedListScoreState.h"
#include "../internal/ParticleGrid.h"
#include "../internal/kernel_version_info.h"
#include "MaxChangeScoreState.h"

#include <vector>
#include <limits>

namespace IMP
{

//! This class maintains a list of non-bonded pairs of particles
/** \note If no value for the radius key is specified, all radii are
    considered to be zero.

    \note The radius is currently assumed not to change. This could
    be fixed later.

    \todo The structure is slightly dumb about rebuilding and will 
    rebuild the whole list of any of the grids become invalidated.
    This could be improved as each piece is computed separately (so
    they could be cached).

    \ingroup restraint
 */
class IMPDLLEXPORT AllNonbondedListScoreState:
    public NonbondedListScoreState
{
  typedef NonbondedListScoreState P;
protected:
  //! \internal
  struct Bin
  {
    internal::ParticleGrid *grid;
    Float rmax;
    Bin(): grid(NULL), rmax(-1){}
    Bin(const Bin &o): grid(o.grid), rmax(o.rmax){}
  };
  std::vector<Bin> bins_;

  //! \internal
  void rebuild_nbl(Float cut);

  void repartition_points(const Particles &ps, std::vector<Bin> &out);

  float side_from_r(float r) const;

  void generate_nbl(const Bin &particle_bin, const Bin &grid_bin, float cut);

  void cleanup(std::vector<Bin> &bins);

public:
  /**
     \param[in] ps A list of particles to use.
     \param[in] radius The key to use to get the radius
   */ 
  AllNonbondedListScoreState(FloatKey radius,
                             const Particles &ps=Particles());
  ~AllNonbondedListScoreState();
  IMP_SCORE_STATE(internal::kernel_version_info)

  void set_particles(const Particles &ps);

  //! Add a few particles to the nonbonded list
  /** Note that this invalidates the nonbonded list.
      \todo We could just add newly created pairs to the nonbonded list.
   */
  void add_particles(const Particles &ps);

  //! Return a list of all the particles used
  Particles get_particles() const;
};

} // namespace IMP

#endif  /* __IMP_ALL_NONBONDED_LIST_SCORE_STATE_H */
