/**
 *  \file AllSphereNonbondedListScoreState.h
 *  \brief Allow iteration through pairs of a set of spheres.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_ALL_SPHERE_NONBONDED_LIST_SCORE_STATE_H
#define __IMP_ALL_SPHERE_NONBONDED_LIST_SCORE_STATE_H

#include "NonbondedListScoreState.h"
#include "../internal/ParticleGrid.h"
#include "MaxChangeScoreState.h"

#include <vector>
#include <limits>

namespace IMP
{

//! This class maintains a list of non-bonded pairs of spheres
/** \note The radius is currently assumed not to change. This could
    be fixed later.
    \note Points whose coordinates are not optimized are assumed to 
    stay that way (and vice
    versa, although that direction doesn't matter so much).
    \note The structure is slightly dumb about rebuilding and will 
    rebuild the whole list of any of the grids become invalidated.
    This could be improved as each piece is computed separately (so
    they could be cached).

    \ingroup restraint
 */
class IMPDLLEXPORT AllSphereNonbondedListScoreState:
    public NonbondedListScoreState
{
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
  std::vector<Bin> fixed_bins_;
  FloatKey rk_;

  //! \internal
  void rebuild_nbl(Float cut);

  void repartition_points(const Particles &ps, std::vector<Bin> &out);

  float side_from_r(float r) const {return r*1.6;}

  void generate_nbl(const Bin &particle_bin, const Bin &grid_bin, float cut);

  void cleanup(std::vector<Bin> &bins);

public:
  /**
     \param[in] ps A list of particles to use.
     \param[in] radius The key to use to get the radius
   */
  AllSphereNonbondedListScoreState(const Particles &ps, FloatKey radius);
  ~AllSphereNonbondedListScoreState();
  IMP_SCORE_STATE("0.5", "Daniel Russel");

  void set_particles(const Particles &ps);
};

} // namespace IMP

#endif  /* __IMP_ALL_SPHERE_NONBONDED_LIST_SCORE_STATE_H */
