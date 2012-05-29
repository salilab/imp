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



//! Define the basics needed for a ScoreState
/** In addition to the methods done by IMP_OBJECT, it declares
    - IMP::ScoreState::do_before_evaluate()
    - IMP::ScoreState::do_after_evaluate()
    - IMP::ScoreState::get_input_containers()
    - IMP::ScoreState::get_output_containers()
    - IMP::ScoreState::get_input_particles()
    - IMP::ScoreState::get_output_particles()
*/
#define IMP_SCORE_STATE(Name)                                   \
  protected:                                                    \
  virtual void do_before_evaluate();                            \
  virtual void do_after_evaluate(DerivativeAccumulator *da);    \
  virtual ContainersTemp get_input_containers() const;          \
  virtual ContainersTemp get_output_containers() const;         \
  virtual ParticlesTemp get_input_particles() const;            \
  virtual ParticlesTemp get_output_particles() const;           \
  IMP_OBJECT(Name)


//! Define the basics needed for a ScoreState
/** In addition to the methods done by IMP_OBJECT, it declares
    - IMP::Constraint::do_update_attributes()
    - IMP::Constraint::do_update_derivatives()
*/
#define IMP_CONSTRAINT(Name)                                            \
  protected:                                                            \
  void do_update_attributes();                                          \
  void do_update_derivatives(DerivativeAccumulator *da);                \
  IMP_NO_DOXYGEN(void do_before_evaluate() { Name::do_update_attributes();}) \
  IMP_NO_DOXYGEN(void do_after_evaluate(DerivativeAccumulator*da) {     \
      if (da) Name::do_update_derivatives(da);})                        \
  virtual ContainersTemp get_input_containers() const;                  \
  virtual ContainersTemp get_output_containers() const;                 \
  virtual ParticlesTemp get_input_particles() const;                    \
  virtual ParticlesTemp get_output_particles() const;                   \
  IMP_OBJECT(Name)


#endif  /* IMPKERNEL_SCORE_STATE_MACROS_H */
