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
class IMPMISCEXPORT StateAdaptor : public ScoreState {
  IMP::base::PointerMember<OptimizerState> before_, after_;

 public:
  //! set the before and after states
  StateAdaptor(Model *m, OptimizerState *before, OptimizerState *after);
  virtual void do_before_evaluate() IMP_OVERRIDE;
  virtual void do_after_evaluate(DerivativeAccumulator *da) IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_outputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(StateAdaptor);
};

IMPMISC_END_NAMESPACE

#endif /* IMPMISC_STATE_ADAPTOR_H */
