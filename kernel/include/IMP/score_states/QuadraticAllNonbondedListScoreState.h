/**
 *  \file QuadraticAllNonbondedListScoreState.h
 *  \brief Allow iteration through pairs of a set of spheres.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_QUADRATIC_ALL_NONBONDED_LIST_SCORE_STATE_H
#define __IMP_QUADRATIC_ALL_NONBONDED_LIST_SCORE_STATE_H

#include "NonbondedListScoreState.h"
#include "../internal/kernel_version_info.h"
#include "QuadraticNonbondedListScoreState.h"

#include <vector>
#include <limits>

namespace IMP
{

//! This class maintains a list of non-bonded pairs of spheres
/** \note Points whose coordinates are not optimized are assumed to 
    stay that way and are pairs involving only fixed points are skipped

    \note QuadraticBipartiteNonbondedListScoreState is basically an
    implementation detail for performance analysis and should not
    be used by end users.
 */
class IMPDLLEXPORT QuadraticAllNonbondedListScoreState:
    public QuadraticNonbondedListScoreState
{
  typedef QuadraticNonbondedListScoreState P;
  Particles fixed_;

  //! \internal
  void rebuild_nbl(Float cut);

public:
  /**
     \param[in] ps A list of particles to use.
     \param[in] radius The key to use to get the radius
   */
  QuadraticAllNonbondedListScoreState(FloatKey radius,
                                      const Particles &ps= Particles());
  ~QuadraticAllNonbondedListScoreState();
  IMP_SCORE_STATE(internal::kernel_version_info)

  void set_particles(const Particles &ps);
};

} // namespace IMP

#endif  /* __IMP_QUADRATIC_ALL_NONBONDED_LIST_SCORE_STATE_H */
