/**
 *  \file BipartiteNonbondedListScoreState.h
 *  \brief Allow iteration through pairs of a set of atoms.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_BIPARTITE_NONBONDED_LIST_SCORE_STATE_H
#define __IMP_BIPARTITE_NONBONDED_LIST_SCORE_STATE_H

#include "QuadraticBipartiteNonbondedListScoreState.h"

namespace IMP
{
//! Maintain a nonbonded list between two disjoint sets.
/**
   \note If no value for the radius key is specified, all radii are
   considered to be zero.

   \ingroup restraint
 */
class BipartiteNonbondedListScoreState:
    public QuadraticBipartiteNonbondedListScoreState {
  typedef QuadraticBipartiteNonbondedListScoreState P;
 public:
  BipartiteNonbondedListScoreState(FloatKey rk,
                                   const Particles &ps0,
                                   const Particles &ps1): P(rk, ps0, ps1){}
  BipartiteNonbondedListScoreState(FloatKey rk): P(rk){}
};

} // namespace IMP

#endif  /* __IMP_BIPARTITE_NONBONDED_LIST_SCORE_STATE_H */
