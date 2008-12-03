/**
 *  \file BondedListScoreState.h
 *  \brief Allow iteration through pairs of a set of atoms.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_BONDED_LIST_SCORE_STATE_H
#define IMPCORE_BONDED_LIST_SCORE_STATE_H

#include "core_exports.h"

#include <IMP/ScoreState.h>
#include <IMP/Index.h>
#include <IMP/Particle.h>

IMPCORE_BEGIN_NAMESPACE

class BondedListScoreState;
//! Used to identify the BondedListScoreState in the NonbondedListScoreState
typedef Index<BondedListScoreState> BondedListIndex;

//! This abstract class maintains a list of bonded pairs.
/** \deprecated Use a ParticlePairContainer of some sort instead.

 */
class IMPCOREEXPORT BondedListScoreState: public ScoreState
{
 public:
  BondedListScoreState(){}
  virtual ~BondedListScoreState(){}

  //! Return true if the two particles are bonded
  virtual bool are_bonded(Particle *a, Particle *b) const =0;
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_BONDED_LIST_SCORE_STATE_H */
