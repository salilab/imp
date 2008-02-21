/**
 *  \file BipartiteNonbondedListScoreState.h
 *  \brief Allow iteration through pairs of a set of atoms.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_BIPARTITE_NONBONDED_LIST_SCORE_STATE_H
#define __IMP_BIPARTITE_NONBONDED_LIST_SCORE_STATE_H

#include "NonbondedListScoreState.h"
#include "../internal/ParticleGrid.h"

#include <vector>
#include <limits>

namespace IMP
{

class BondedListScoreState;

//! This class maintains a list of non-bonded pairs between two sets.
/** The class works roughly like the NonbondedListScoreState except
    only pairs where one particle is taken from each set are returned.
    \ingroup restraint
 */
class IMPDLLEXPORT BipartiteNonbondedListScoreState:
    public NonbondedListScoreState
{
  typedef NonbondedListScoreState P;
  internal::ObjectPointer<MaxChangeScoreState, true> mc_;
  internal::ParticleGrid grid_;

  virtual void rebuild_nbl(float cut);
public:
  BipartiteNonbondedListScoreState(const Particles &ps0,
                                   const Particles &ps1,
                                   float target_side);

  IMP_SCORE_STATE("0.5", "Daniel Russel");

  void set_particles(const Particles &ps0, const Particles &ps1);
};

} // namespace IMP

#endif  /* __IMP_BIPARTITE_NONBONDED_LIST_SCORE_STATE_H */
