/**
 *  \file OptimizerState.h   \brief Shared optimizer state.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_OPTIMIZER_STATE_H
#define IMP_OPTIMIZER_STATE_H

#include "config.h"
#include "VersionInfo.h"
#include "RefCountedObject.h"
#include "Pointer.h"
#include "Optimizer.h"

#include <iostream>

IMP_BEGIN_NAMESPACE

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
class IMPEXPORT OptimizerState : public RefCountedObject
{
  friend class Optimizer;
  void set_optimizer(Optimizer* optimizer);

public:
  OptimizerState();

  //! Called when the Optimizer accepts a new conformation
  virtual void update() = 0;

  virtual void show(std::ostream& out = std::cout) const;

  virtual VersionInfo get_version_info() const {
    return VersionInfo("unknown", "unknown");
  }

  Optimizer *get_optimizer() const {
    IMP_assert(optimizer_,
               "Must call set_optimizer before get_optimizer on state");
    return optimizer_.get();
  }
  IMP_REF_COUNTED_DESTRUCTOR(OptimizerState)
protected:
  WeakPointer<Optimizer> optimizer_;
};

IMP_OUTPUT_OPERATOR(OptimizerState);

IMP_END_NAMESPACE

#endif  /* IMP_OPTIMIZER_STATE_H */
