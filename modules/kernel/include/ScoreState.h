/**
 *  \file IMP/kernel/ScoreState.h   \brief Shared score state.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_SCORE_STATE_H
#define IMPKERNEL_SCORE_STATE_H

#include <IMP/kernel/kernel_config.h>
#include "WeakPointer.h"
#include "DerivativeAccumulator.h"
#include "utility.h"
#include "ModelObject.h"
#include "base_types.h"
#include <IMP/base/check_macros.h>
#include <IMP/base/deprecation_macros.h>
#include <IMP/base/ref_counted_macros.h>
#include <iostream>

IMPKERNEL_BEGIN_NAMESPACE
//! ScoreStates maintian invariants in the Model.
/** ScoreStates allow actions to be taken before and after the restraint
    evaluation process. Such code can be used to, for example:
    - log the optimization process
    - maintain constraints (see Constraint)

    ScoreStates have two methods which are called during
    the Model::evaluate() function
    - before_evaluate() which is allowed to change the contents of
    containers and the value of attributes of particles and
    - after_evaluate() which can change particle derivatives

    The Model uses information that the ScoreState reports about
    its input and output containers and particles to determine a safe
    order in which all the ScoreState objects registered in the model
    can be applied. That is, the Model will ensure that a ScoreState
    that has Particle \c A in its output list is applied before a
    ScoreState that has \c A in its input list.
    For ScoreState::before_evaluate(), Input and output lists are
    returned by the respective get_input_* and get_output_* calls.
    For ScoreState::after_evaluate() they are reversed (see note below).


    \note If no acceptable order exists, an exception will be thrown
    and the set of ScoreState objects creating the loop will be
    reported.

    \note The input and output sets for the ScoreState::after_evaluate()
    functions are assumed to be the reverse of the ScoreState::before_evaluate()
    functions. As a result, the ScoreStates are applied in opposite order
    after evaluate. If you have a ScoreState for which this is not true,
    consider splitting it into two parts.
 */
class IMPKERNELEXPORT ScoreState : public ModelObject
{
public:
#ifndef IMP_DOXYGEN
  ScoreState(std::string name="ScoreState %1%");
#endif
  ScoreState(Model *m, std::string name="ScoreState %1%");
  //! Force update of the structure.
  void before_evaluate();

  //! Do post evaluation work if needed
  void after_evaluate(DerivativeAccumulator *accpt);

protected:
  // Update the state given the current state of the model.
  /* This is also called prior to every calculation of the model score.
      It should be implemented by ScoreStates in order to provide functionality.

      \note This can't have the same name as the public function due to the
      way C++ handles overloading and name lookups--if only one is implemented
      in the child class it will only find that one.
   */
  virtual void do_before_evaluate() = 0;

  // Do any necessary updates after the model score is calculated.
  /* \param[in] accpt The object used to scale derivatives in the score
                       calculation, or nullptr if derivatives were not
                       requested.
   */
  virtual void do_after_evaluate(DerivativeAccumulator *accpt)=0;

  IMP_REF_COUNTED_NONTRIVIAL_DESTRUCTOR(ScoreState);

public:
#if IMP_HAS_DEPRECATED
  /** \deprecated use get_inputs() instead.*/
  IMP_DEPRECATED_WARN ParticlesTemp get_input_particles() const;
  /** \deprecated use get_inputs() instead.*/
  IMP_DEPRECATED_WARN ContainersTemp get_input_containers() const;
  /** \deprecated use get_outputs() instead.*/
  IMP_DEPRECATED_WARN ParticlesTemp get_output_particles() const;
  /** \deprecated use get_outputs() instead.*/
  IMP_DEPRECATED_WARN ContainersTemp get_output_containers() const;
#endif
protected:
  virtual void do_update_dependencies() IMP_OVERRIDE;
 private:

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
public:
#endif
  int order_;
};

/** Return the passed list of score states ordered based on how they need to
    be ordered during update calls.*/
IMPKERNELEXPORT ScoreStatesTemp get_update_order( ScoreStatesTemp input);

IMPKERNEL_END_NAMESPACE


#endif  /* IMPKERNEL_SCORE_STATE_H */
