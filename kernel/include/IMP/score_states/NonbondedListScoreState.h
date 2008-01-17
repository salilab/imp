/**
 *  \file NonbondedListScoreState.h
 *  \brief Allow iteration through pairs of a set of atoms.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_NONBONDED_LIST_SCORE_STATE_H
#define __IMP_NONBONDED_LIST_SCORE_STATE_H

#include <vector>
#include <limits>
#include "../ScoreState.h"
#include "BondedListScoreState.h"

namespace IMP
{

class BondedListScoreState;

//! This class maintains a list of non-bonded pairs.
/** The distance cutoff is an optimization hint rather than a
    strict cutoff. That is, the NonbondedListScoreState may
    choose to ignore pairs above that cutoff, but may not.
 */
class IMPDLLEXPORT NonbondedListScoreState: public ScoreState
{
  Particles ps_;
  typedef std::vector<std::pair<Particle*, Particle*> > NBL;
  NBL nbl_;
  float dist_cutoff_;

  void rescan();

public:
  NonbondedListScoreState(const Particles &ps,
                          float dist_cutoff
                          = std::numeric_limits<Float>::max());
  virtual ~NonbondedListScoreState() {}
  IMP_CONTAINER(BondedListScoreState, bonded_list_score_state,
                BondedListIndex);
public:
  IMP_SCORE_STATE("0.5", "Daniel Russel");

  void set_particles(const Particles &ps);

  //! This iterates through the pairs of non-bonded particles
  /** \precondition update() must be called first for this to be valid.
   */
  typedef NBL::const_iterator NonbondedIterator;
  NonbondedIterator nonbonded_begin() const {
    return nbl_.begin();
  }
  NonbondedIterator nonbonded_end() const {
    return nbl_.end();
  }
};

} // namespace IMP

#endif  /* __IMP_NONBONDED_LIST_SCORE_STATE_H */
