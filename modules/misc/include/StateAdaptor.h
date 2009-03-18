/**
 *  \file StateAdaptor.h
 *  \brief XXXXXXXXXXXXXX
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPMISC_STATE_ADAPTOR_H
#define IMPMISC_STATE_ADAPTOR_H

#include "config.h"

#include "internal/version_info.h"
#include <IMP/OptimizerState.h>
#include <IMP/ScoreState.h>

IMPMISC_BEGIN_NAMESPACE

//! Allow OptimizerStates to be used as ScoreStates
/** You can have up to two, one for before, and one for after.
 */
class IMPMISCEXPORT StateAdaptor: public ScoreState
{
  Pointer<OptimizerState> before_, after_;
public:
  //! set the before and after states
  StateAdaptor(OptimizerState *before, OptimizerState *after);

  IMP_SCORE_STATE(StateAdaptor, internal::version_info)
};


IMPMISC_END_NAMESPACE

#endif  /* IMPMISC_STATE_ADAPTOR_H */
