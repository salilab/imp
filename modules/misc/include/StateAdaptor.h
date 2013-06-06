/**
 *  \file IMP/misc/StateAdaptor.h
 *  \brief XXXXXXXXXXXXXX
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPMISC_STATE_ADAPTOR_H
#define IMPMISC_STATE_ADAPTOR_H

#include <IMP/misc/misc_config.h>

#include <IMP/OptimizerState.h>
#include <IMP/ScoreState.h>
#include <IMP/score_state_macros.h>

IMPMISC_BEGIN_NAMESPACE

//! Allow OptimizerStates to be used as ScoreStates
/** You can have up to two, one for before, and one for after.
 */
class IMPMISCEXPORT StateAdaptor: public ScoreState
{
  IMP::base::OwnerPointer<OptimizerState> before_, after_;
public:
  //! set the before and after states
  StateAdaptor(OptimizerState *before, OptimizerState *after);

  IMP_SCORE_STATE(StateAdaptor);
};


IMPMISC_END_NAMESPACE

#endif  /* IMPMISC_STATE_ADAPTOR_H */
