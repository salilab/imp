/**
 *  \file OptimizerState.h   \brief Shared optimizer state.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_OPTIMIZER_STATE_H
#define __IMP_OPTIMIZER_STATE_H

#include <iostream>

#include "IMP_config.h"
#include "VersionInfo.h"
#include "internal/Object.h"
#include "internal/ObjectPointer.h"
#include "Optimizer.h"

namespace IMP
{

class Optimizer;

//! Shared optimizer state.
/** The OptimizerState update method is called each time the Optimizer commits
    to a new set of coordinates.
 */
class IMPDLLEXPORT OptimizerState : public internal::Object
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
  internal::ObjectPointer<Optimizer, false> optimizer_;
};


inline std::ostream &operator<<(std::ostream &out, const OptimizerState &s)
{
  s.show(out);
  return out;
}

} // namespace IMP

#endif  /* __IMP_OPTIMIZER_STATE_H */
