/**
 *  \file IMP/score_state_macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_SCORE_STATE_MACROS_H
#define IMPKERNEL_SCORE_STATE_MACROS_H
#include "kernel_config.h"
#include <IMP/base/value_macros.h>
#include "ScoreState.h"
#include "container_base.h"
#include "input_output_macros.h"



//! Use IMP_SCORE_STATE_2()
#define IMP_SCORE_STATE(Name)                                           \
  protected:                                                            \
  virtual void do_before_evaluate();                                    \
  virtual void do_after_evaluate(DerivativeAccumulator *da);            \
  IMP_MODEL_OBJECT_BACKWARDS_MACRO_INPUTS;                              \
  IMP_MODEL_OBJECT_BACKWARDS_MACRO_OUTPUTS;                             \
  IMP_OBJECT(Name)

//! Define the basics needed for a ScoreState
/** In addition to the methods done by IMP_OBJECT, it declares
    - IMP::ScoreState::do_before_evaluate()
    - IMP::ScoreState::do_after_evaluate()
    - IMP::ModelObject::do_get_inputs()
    - IMP::ModelObject::do_get_outputs()
*/
#define IMP_SCORE_STATE_2(Name)                                         \
  protected:                                                            \
  virtual void do_before_evaluate();                                    \
  virtual void do_after_evaluate(DerivativeAccumulator *da);            \
  virtual ModelObjectsTemp do_get_inputs() const;                       \
  virtual ModelObjectsTemp do_get_outputs() const;                      \
  IMP_OBJECT(Name)

//! Use IMP_CONSTRAINT_2()
#define IMP_CONSTRAINT(Name)                                            \
  protected:                                                            \
  void do_update_attributes();                                          \
  void do_update_derivatives(DerivativeAccumulator *da);                \
  IMP_NO_DOXYGEN(void do_before_evaluate() { Name::do_update_attributes();}) \
  IMP_NO_DOXYGEN(void do_after_evaluate(DerivativeAccumulator*da) {     \
      if (da) Name::do_update_derivatives(da);})                        \
  IMP_MODEL_OBJECT_BACKWARDS_MACRO_INPUTS;                              \
  IMP_MODEL_OBJECT_BACKWARDS_MACRO_OUTPUTS;                             \
  IMP_OBJECT(Name)

//! Define the basics needed for a ScoreState
/** In addition to the methods done by IMP_OBJECT, it declares
    - IMP::Constraint::do_update_attributes()
    - IMP::Constraint::do_update_derivatives()
    - IMP::ModelObject::do_get_inputs()
    - IMP::ModelObject::do_get_outputs()
*/
#define IMP_CONSTRAINT_2(Name)                                          \
  protected:                                                            \
  void do_update_attributes();                                          \
  void do_update_derivatives(DerivativeAccumulator *da);                \
  IMP_NO_DOXYGEN(void do_before_evaluate() { Name::do_update_attributes();}) \
  IMP_NO_DOXYGEN(void do_after_evaluate(DerivativeAccumulator*da) {     \
      if (da) Name::do_update_derivatives(da);})                        \
  virtual ModelObjectsTemp do_get_inputs() const;                       \
  virtual ModelObjectsTemp do_get_outputs() const;                      \
  IMP_OBJECT(Name)


#endif  /* IMPKERNEL_SCORE_STATE_MACROS_H */
