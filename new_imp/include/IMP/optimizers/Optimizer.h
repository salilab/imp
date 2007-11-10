/**
 *  \file Optimizer.h     \brief Base class for all optimizers.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_OPTIMIZER_H
#define __IMP_OPTIMIZER_H

#include "../IMP_config.h"
#include "../Base_Types.h"
#include "../Model.h"

namespace IMP
{

//! Base class for all optimizers
class IMPDLLEXPORT Optimizer
{
public:
  Optimizer();
  virtual ~Optimizer();

  //! Optimize the model and return the final score.
  virtual Float optimize(Model& model, int max_steps, Float threshold=0.0) = 0;

  virtual std::string version() const {
    return "unknown";
  }
  virtual std::string last_modified_by() const {
    return "unknown";
  }
};

} // namespace IMP

#endif  /* __IMP_OPTIMIZER_H */
