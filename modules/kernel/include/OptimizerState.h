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

//! Shared optimizer state.
/** The OptimizerState update method is called each time the Optimizer commits
    to a new set of coordinates. Optimizer states may change the values of
    particle attributes. However, changes to whether an attribute is optimized
    or not may not be picked up by the Optimizer until the next call to
    optimize.

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
  OptimizerState(Model *m, std::string name);
  /** \deprecated_at{2.1} Use the constructor that takes a Model. */
  IMPKERNEL_DEPRECATED_FUNCTION_DECL(2.1)
  OptimizerState(std::string name = "OptimizerState %1%");

  //! Called when the Optimizer accepts a new conformation
  /** Overriding this method is deprecated, override do_update() instead.
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

  /** If is often useful to have the OptimizerState only take action on a subset
      of the steps. */
  void set_period(unsigned int p);
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
  virtual void do_update(unsigned int call_number) {update();}

  virtual void do_set_is_optimizing(bool) {}

  virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE {
    return ModelObjectsTemp();
  }
  virtual ModelObjectsTemp do_get_outputs() const IMP_OVERRIDE IMP_FINAL {
    return ModelObjectsTemp();
  }
  base::UncheckedWeakPointer<Optimizer> optimizer_;
};

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_OPTIMIZER_STATE_H */
