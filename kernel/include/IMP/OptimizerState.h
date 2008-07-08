/**
 *  \file OptimizerState.h   \brief Shared optimizer state.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_OPTIMIZER_STATE_H
#define __IMP_OPTIMIZER_STATE_H

#include "IMP_config.h"
#include "VersionInfo.h"
#include "RefCountedObject.h"
#include "Pointer.h"
#include "Optimizer.h"

#include <iostream>

namespace IMP
{

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
class IMPDLLEXPORT OptimizerState : public Object
{
  friend class Optimizer;
  void set_optimizer(Optimizer* optimizer);

public:
  OptimizerState();
  virtual ~OptimizerState();

  // Update the state given the current state of the optimizer
  virtual void update() = 0;

  //! Show the OptimizerState
  /** Show can print out multiple lines and should end 
      in a newline.
  */
  virtual void show(std::ostream& out = std::cout) const;

  //! \return version and authorship information.
  virtual VersionInfo get_version_info() const {
    return VersionInfo("unknown", "unknown");
  }

  //! return the stored optimizer
  Optimizer *get_optimizer() const {
    IMP_assert(optimizer_,
               "Must call set_optimizer before get_optimizer on state");
    return optimizer_.get();
  }
protected:
  //! Stored optimizer
  Pointer<Optimizer> optimizer_;
};

IMP_OUTPUT_OPERATOR(OptimizerState);

} // namespace IMP

#endif  /* __IMP_OPTIMIZER_STATE_H */
