/**
 *  \file BondedListScoreState.h
 *  \brief Allow iteration through pairs of a set of atoms.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_BONDED_LIST_SCORE_STATE_H
#define __IMP_BONDED_LIST_SCORE_STATE_H

#include <set>
#include "../ScoreState.h"
#include "../Index.h"
#include "../Particle.h"

namespace IMP
{
class BondedListScoreState;
typedef Index<BondedListScoreState> BondedListIndex;

//! This abstract class maintains a list of bonded pairs.
class IMPDLLEXPORT BondedListScoreState: public ScoreState
{
  struct BP: public std::pair<Particle*, Particle *>
  {
    typedef BP This;
    typedef std::pair<Particle*, Particle *> P;
    BP(): P(NULL, NULL) {}
    bool is_default() const {
      return first == NULL && second == NULL;
    }
    BP(Particle* a, Particle *b): P(std::min(a, b), std::max(a, b)) {}
    IMP_COMPARISONS_2(first, second);
  };
  //std::set<BP> bonds_;

public:
  BondedListScoreState() {}
  virtual ~BondedListScoreState() {}
  //IMP_SCORE_STATE("0.5", "Daniel Russel");

  virtual bool are_bonded(Particle *a, Particle *b) const = 0;
#if 0
  //! This iterates through the pairs of non-bonded particles
  /**
     \precondition update() must be called first for this to be valid.
   */
  typedef std::set<BP>::const_iterator BondedIterator;
  BondedIterator bonded_begin() const {
    return bonds_.begin();
  }
  BondedIterator bonded_end() const {
    return bonds_.end();
  }
#endif
};

} // namespace IMP

#endif  /* __IMP_BONDED_LIST_SCORE_STATE_H */
