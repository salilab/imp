/**
 *  \file BipartiteNonbondedListScoreState.h
 *  \brief Allow iteration through pairs of a set of atoms.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_BIPARTITE_NONBONDED_LIST_SCORE_STATE_H
#define __IMP_BIPARTITE_NONBONDED_LIST_SCORE_STATE_H

#include <vector>
#include <limits>
#include "../ScoreState.h"
#include "NonbondedListScoreState.h"

namespace IMP
{

class BondedListScoreState;

//! This class maintains a list of non-bonded pairs between two sets.
/** The class works roughly like the NonbondedListScoreState except
    only pairs where one particle is taken from each set are returned.
 */
class IMPDLLEXPORT BipartiteNonbondedListScoreState:
    public NonbondedListScoreState
{
  typedef NonbondedListScoreState P;
  std::auto_ptr<MaxChangeScoreState> mc_;

  virtual void rescan(float cut);
  void set_particles(const Particles &ps0) {
    // hide the parent's version
  }
public:
  BipartiteNonbondedListScoreState(const Particles &ps0,
                                   const Particles &ps1,
                                   float target_side);
  virtual ~BipartiteNonbondedListScoreState();

  IMP_SCORE_STATE("0.5", "Daniel Russel");

  void set_particles(const Particles &ps0, const Particles &ps1);
  const Particles &get_particles() const {
    return mc_->get_particles();
  }
};

} // namespace IMP

#endif  /* __IMP_BIPARTITE_NONBONDED_LIST_SCORE_STATE_H */
