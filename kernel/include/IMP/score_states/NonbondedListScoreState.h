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
class IMPDLLEXPORT NonbondedListScoreState: public ScoreState
{
 protected:
  Particles ps_;
  typedef std::vector<std::pair<Particle*, Particle*> > NBL;
  NBL nbl_;

  void rescan();
  void audit_particles(const Particles &ps) const;
  void propagate_set_particles(const Particles &aps);
  void propagate_update();
  void add_if_nonbonded(Particle *a, Particle *b);
public:
  NonbondedListScoreState(const Particles &ps);
  virtual ~NonbondedListScoreState();
  IMP_CONTAINER(BondedListScoreState, bonded_list,
                BondedListIndex);
  // kind of evil hack to make the names better
  // perhaps the macro should be made more flexible
  typedef BondedListScoreStateIterator BondedListIterator;
public:
  IMP_SCORE_STATE("0.5", "Daniel Russel");

  void set_particles(const Particles &ps);

  typedef NBL::const_iterator NonbondedIterator;

  //! This iterates through the pairs of non-bonded particles
  /** \param[in] cutoff The state may ignore pairs which are futher
      apart than the cutoff.
      \precondition update() must be called first for this to be valid.
  */
  NonbondedIterator nonbonded_begin(Float cutoff
                                    =std::numeric_limits<Float>::max()) const {
    return nbl_.begin();
  }
  NonbondedIterator nonbonded_end(Float cutoff
                                  =std::numeric_limits<Float>::max()) const {
    return nbl_.end();
  }
};

} // namespace IMP

#endif  /* __IMP_NONBONDED_LIST_SCORE_STATE_H */
