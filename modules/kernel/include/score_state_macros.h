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



//! \deprecated_at{2.1} Declare the needed functions directly
#define IMP_SCORE_STATE(Name)                                           \
  IMPKERNEL_DEPRECATED_MACRO(2.1, "Declare the needed functions directly."); \
  protected:                                                            \
  virtual void do_before_evaluate() IMP_OVERRIDE;                       \
  virtual void do_after_evaluate(DerivativeAccumulator *da) IMP_OVERRIDE; \
  IMP_MODEL_OBJECT_BACKWARDS_MACRO_INPUTS;                              \
  IMP_MODEL_OBJECT_BACKWARDS_MACRO_OUTPUTS;                             \
  IMP_OBJECT_NO_WARNING(Name)


//! \deprecated_at{2.1} Declare the needed functions directly
#define IMP_CONSTRAINT(Name)                                            \
  IMPKERNEL_DEPRECATED_MACRO(2.1, "Declare the needed functions directly."); \
  protected:                                                            \
  virtual void do_update_attributes() IMP_OVERRIDE;                     \
  virtual void do_update_derivatives(DerivativeAccumulator *da) IMP_OVERRIDE; \
  IMP_MODEL_OBJECT_BACKWARDS_MACRO_INPUTS;                              \
  IMP_MODEL_OBJECT_BACKWARDS_MACRO_OUTPUTS;                             \
  IMP_OBJECT_NO_WARNING(Name)

//! \deprecated_at{2.1} Declare the needed functions directly
#define IMP_SCORE_STATE_2(Name)                                         \
  IMPKERNEL_DEPRECATED_MACRO(2.1, "Declare the needed functions directly."); \
  protected:                                                            \
  virtual void do_before_evaluate() IMP_OVERRIDE;                       \
  virtual void do_after_evaluate(DerivativeAccumulator *da) IMP_OVERRIDE; \
  virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;          \
  virtual ModelObjectsTemp do_get_outputs() const IMP_OVERRIDE;         \
  IMP_OBJECT_NO_WARNING(Name)


//! \deprecated_at{2.1} Declare the needed functions directly
#define IMP_CONSTRAINT_2(Name)                                          \
  IMPKERNEL_DEPRECATED_MACRO(2.1, "Declare the needed functions directly."); \
  protected:                                                            \
  void do_update_attributes();                                          \
  void do_update_derivatives(DerivativeAccumulator *da);                \
  virtual ModelObjectsTemp do_get_inputs() const;                       \
  virtual ModelObjectsTemp do_get_outputs() const;                      \
  IMP_OBJECT_NO_WARNING(Name)

#endif  /* IMPKERNEL_SCORE_STATE_MACROS_H */
