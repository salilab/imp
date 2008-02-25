/**
 *  \file AllNonbondedListScoreState.h
 *  \brief Allow iteration through pairs of a set of particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_ALL_NONBONDED_LIST_SCORE_STATE_H
#define __IMP_ALL_NONBONDED_LIST_SCORE_STATE_H

#include "NonbondedListScoreState.h"
#include "../internal/kernel_version_info.h"

#include <vector>
#include <limits>

namespace IMP
{

//! This class maintains a list of non-bonded pairs.
/** The nonbonded list returns all nonboned pairs in the set of points.
    \ingroup restraint
 */
class IMPDLLEXPORT AllNonbondedListScoreState: public NonbondedListScoreState
{
protected:
  internal::ParticleGrid grid_;

  virtual void rebuild_nbl(float cut);

public:
  /**
     \param[in] ps A list of particles to use.
     \param[in] tvs A suggested size for the voxel side.
   */
  AllNonbondedListScoreState(const Particles &ps, float tvs=1);

  IMP_SCORE_STATE(internal::kernel_version_info)

  void set_particles(const Particles &ps);
};

} // namespace IMP

#endif  /* __IMP_ALL_NONBONDED_LIST_SCORE_STATE_H */
