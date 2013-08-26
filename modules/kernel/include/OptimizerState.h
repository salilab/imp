/**
 *  \file IMP/kernel/OptimizerState.h   \brief Shared optimizer state.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_OPTIMIZER_STATE_H
#define IMPKERNEL_OPTIMIZER_STATE_H

#include <IMP/kernel/kernel_config.h>
#include "VersionInfo.h"
#include "ModelObject.h"
#include <IMP/base/WeakPointer.h>
#include <IMP/base/Object.h>

#include <iostream>

IMPKERNEL_BEGIN_NAMESPACE

class Optimizer;

//! Shared optimizer state that is invoked upon commitment of new coordinates.
/** An OptimizerState update() method is called every time that an
    owning Optimizer commits to a new set of coordinates. The update()
    method, in turn, invokes do_update(), which can be overridden by
    inheriting classes.

    @note An OptimizerState may have periodicity by its set_period() method

    @note An OptimizerState is added to an Optimizer object by calling
          Optimizer::add_optimizer_state().

    @note Optimizer states may change the values of particle
          attributes. However, changes to whether an attribute is optimized
          or not may not be picked up by the Optimizer until the next call
          to optimize.

    \note When logging is VERBOSE, state should print enough information
          in evaluate to reproduce the the entire flow of data in update. When
          logging is TERSE the restraint should print out only a constant number
          of lines per update call.
 */
class IMPKERNELEXPORT OptimizerState : public ModelObject {
  friend class Optimizer;
  unsigned int period_, call_number_, update_number_;

  void set_optimizer(Optimizer* optimizer);

 public:
  /** constructs an optimizer state whose update() method  is invoked
      every time that a set of model coordinates is committed
      by an optimizer.

      @param m the model to which this optimizer state is associated
      @param name the name of the object
      @note An optimizer state may become periodic via its set_period()
            method.
  */
  OptimizerState(Model *m, std::string name);

  /** \deprecated_at{2.1} Use the constructor that takes a Model. */
  IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.1)
  OptimizerState(std::string name = "OptimizerState %1%");

  //! Called when the Optimizer accepts a new conformation
  /**
      This method is called by owning optimizers every time they commit.
      However, if set_period(p) was invoked, it calls do_update() only
      every p times it is called (by any optimizer).

      @note Overriding this method is deprecated, override do_update() instead.
  */
  virtual void update();

  /** Called with true at the beginning of an optimizing run and with
      false at the end.

      \note Do not override, override do_set_is_optimizing() instead.
  */
  virtual void set_is_optimizing(bool);

  Optimizer* get_optimizer() const {
    IMP_INTERNAL_CHECK(optimizer_,
                       "Must call set_optimizer before get_optimizer on state");
    return optimizer_.get();
  }

  /**
     Causes update() invoke do_update() only every p calls to update()
     instead of the default p=1. Note that this periodicity is shared
     by all optimizers that own this OptimizerState object.

     @param p periodicity
   */
  void set_period(unsigned int p);

  /**
     @return the periodicity of the optimizer (how many calls to update()
             are required to invoke do_update()
  */
  unsigned int get_period() const {return period_;}

  /** Reset the phase to 0 and set the call number to 0 too.*/
  virtual void reset();

  /** Force the optimizer state to perform its action now, ignoring the
      phase.
   */
  void update_always();

  //! Return the number of times update has been called
  unsigned int get_number_of_updates() const { return update_number_; }

  //! Set the counter
  void set_number_of_updates(unsigned int n) { update_number_ = n; }

  IMP_REF_COUNTED_DESTRUCTOR(OptimizerState);

 protected:
  /** This method is called every get_period() update calls. The number of
      times this method has been called since the last reset or start of the
      optimization run is passed.*/
  virtual void do_update(unsigned int ) {update();}

  virtual void do_set_is_optimizing(bool) {}

  virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE {
    return ModelObjectsTemp();
  }
  virtual ModelObjectsTemp do_get_outputs() const IMP_OVERRIDE IMP_FINAL {
    return ModelObjectsTemp();
  }
 private:
  base::UncheckedWeakPointer<Optimizer> optimizer_;
  bool is_optimizing_;
};

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_OPTIMIZER_STATE_H */
