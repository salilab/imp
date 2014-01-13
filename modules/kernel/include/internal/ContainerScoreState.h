/**
 *  \file generic.h    \brief Various important functionality
 *                                       for implementing decorators.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INTERNAL_CONTAINER_SCORE_STATE_H
#define IMPKERNEL_INTERNAL_CONTAINER_SCORE_STATE_H

#include "../Constraint.h"
#include "container_helpers.h"
#include "../score_state_macros.h"
#include <IMP/base/Pointer.h>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

/** A backwards compat score state to ease the transitioning away
    from containers as score states. */
template <class Container>
class ContainerScoreState : public ScoreState {
  base::WeakPointer<Container> back_;

 public:
  ContainerScoreState(Container *back)
      : ScoreState(back->get_model(), back->get_name() + "State"),
        back_(back) {}
  virtual void do_before_evaluate() IMP_OVERRIDE {
    IMP_CHECK_OBJECT(back_);
    return back_->do_score_state_before_evaluate();
  }

  virtual void do_after_evaluate(DerivativeAccumulator *) {}

  virtual ModelObjectsTemp do_get_inputs() const {
    IMP_CHECK_OBJECT(back_);
    return back_->get_score_state_inputs();
  }

  virtual ModelObjectsTemp do_get_outputs() const {
    IMP_CHECK_OBJECT(back_);
    return ModelObjectsTemp(1, back_);
  }
};

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_INTERNAL_CONTAINER_SCORE_STATE_H */
