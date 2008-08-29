/**
 *  \file BondedListScoreState.h
 *  \brief Allow iteration through pairs of a set of atoms.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_BONDED_LIST_SCORE_STATE_H
#define __IMP_BONDED_LIST_SCORE_STATE_H

#include "../ScoreState.h"
#include "../Index.h"
#include "../Particle.h"

namespace IMP
{
class BondedListScoreState;
//! Used to identify the BondedListScoreState in the NonbondedListScoreState
typedef Index<BondedListScoreState> BondedListIndex;

//! This abstract class maintains a list of bonded pairs.
class IMPDLLEXPORT BondedListScoreState: public ScoreState
{
 public:
  BondedListScoreState(){}
  virtual ~BondedListScoreState(){}

  //! Return true if the two particles are bonded
  virtual bool are_bonded(Particle *a, Particle *b) const =0;
};

} // namespace IMP

#endif  /* __IMP_BONDED_LIST_SCORE_STATE_H */
