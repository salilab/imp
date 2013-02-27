/**
 *  \file IMP/kernel/score_state_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_SCORE_STATE_MACROS_H
#define IMPKERNEL_SCORE_STATE_MACROS_H
#include <IMP/kernel/kernel_config.h>
#include <IMP/base/value_macros.h>
#include "ScoreState.h"
#include "container_base.h"
#include "input_output_macros.h"



//! For backwards compatibility
#define IMP_SCORE_STATE(Name)                                           \
  protected:                                                            \
  virtual void do_before_evaluate();                                    \
  virtual void do_after_evaluate(DerivativeAccumulator *da);            \
  IMP_MODEL_OBJECT_BACKWARDS_MACRO_INPUTS;                              \
  IMP_MODEL_OBJECT_BACKWARDS_MACRO_OUTPUTS;                             \
  IMP_OBJECT(Name)


//! For backwards compatibility
#define IMP_CONSTRAINT(Name)                                            \
  protected:                                                            \
  void do_update_attributes();                                          \
  void do_update_derivatives(DerivativeAccumulator *da);                \
  IMP_MODEL_OBJECT_BACKWARDS_MACRO_INPUTS;                              \
  IMP_MODEL_OBJECT_BACKWARDS_MACRO_OUTPUTS;                             \
  IMP_OBJECT(Name)

#if IMP_HAS_DEPRECATED
//! For backwards compatibility
#define IMP_SCORE_STATE_2(Name)                                         \
  protected:                                                            \
  virtual void do_before_evaluate();                                    \
  virtual void do_after_evaluate(DerivativeAccumulator *da);            \
  virtual ModelObjectsTemp do_get_inputs() const;                       \
  virtual ModelObjectsTemp do_get_outputs() const;                      \
  IMP_OBJECT(Name)


//! For backwards compatibility
#define IMP_CONSTRAINT_2(Name)                                          \
  protected:                                                            \
  void do_update_attributes();                                          \
  void do_update_derivatives(DerivativeAccumulator *da);                \
  virtual ModelObjectsTemp do_get_inputs() const;                       \
  virtual ModelObjectsTemp do_get_outputs() const;                      \
  IMP_OBJECT(Name)
#endif

#endif  /* IMPKERNEL_SCORE_STATE_MACROS_H */
