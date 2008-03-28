/**
 *  \file QuadraticBipartiteNonbondedListScoreState.h
 *  \brief Allow iteration through pairs of a set of atoms.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_QUADRATIC_BIPARTITE_NONBONDED_LIST_SCORE_STATE_H
#define __IMP_QUADRATIC_BIPARTITE_NONBONDED_LIST_SCORE_STATE_H

#include "QuadraticNonbondedListScoreState.h"
#include "../internal/ParticleGrid.h"
#include "../internal/kernel_version_info.h"

#include <vector>
#include <limits>

namespace IMP
{

//! This class maintains a list of non-bonded pairs between two sets.
/** The class works roughly like the NonbondedListScoreState except
    only pairs where one particle is taken from each set are returned.

    \note QuadraticBipartiteNonbondedListScoreState is basically an
    implementation detail for performance analysis and should not
    be used by end users.
 */
class IMPDLLEXPORT QuadraticBipartiteNonbondedListScoreState:
    public QuadraticNonbondedListScoreState
{
  typedef QuadraticNonbondedListScoreState P;
  unsigned int na_;

  virtual void rebuild_nbl(float cut);
public:
  QuadraticBipartiteNonbondedListScoreState(FloatKey rk,
                                            const Particles &ps0,
                                            const Particles &ps1);
  QuadraticBipartiteNonbondedListScoreState(FloatKey rk);

  IMP_SCORE_STATE(internal::kernel_version_info)

  void set_particles(const Particles &ps0, const Particles &ps1);
};

} // namespace IMP

#endif  /* __IMP_QUADRATIC_BIPARTITE_NONBONDED_LIST_SCORE_STATE_H */
