/**
 *  \file BondedListScoreState.h
 *  \brief Allow iteration through pairs of a set of atoms.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_BONDED_LIST_SCORE_STATE_H
#define IMPCORE_BONDED_LIST_SCORE_STATE_H

#include "config.h"

#include <IMP/ScoreState.h>
#include <IMP/Particle.h>

IMPCORE_BEGIN_NAMESPACE

class BondedListScoreState;

//! This class is deprecated, use a ParticlePairContainer of some sort instead.
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
