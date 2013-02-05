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
#include "RefCounted.h"
#include "Pointer.h"
#include "WeakPointer.h"
#include "Object.h"

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

    Implementors should see IMP_OPTIMIZER_STATE().
 */
class IMPKERNELEXPORT OptimizerState : public IMP::base::Object
{
  friend class Optimizer;
  void set_optimizer(Optimizer* optimizer);
public:
  OptimizerState(std::string name="OptimizerState %1%");

  //! Called when the Optimizer accepts a new conformation
  virtual void update() = 0;

  /** Called with true at the beginning of an optimizing run and with
      false at the end.*/
  virtual void set_is_optimizing(bool) {}

  Optimizer *get_optimizer() const {
    IMP_INTERNAL_CHECK(optimizer_,
               "Must call set_optimizer before get_optimizer on state");
    return optimizer_.get();
  }
  IMP_REF_COUNTED_DESTRUCTOR(OptimizerState);
protected:
  UncheckedWeakPointer<Optimizer> optimizer_;
};

IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_OPTIMIZER_STATE_H */
