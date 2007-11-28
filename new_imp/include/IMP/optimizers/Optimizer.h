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
#include "../utility.h"

namespace IMP
{

//! Base class for all optimizers
class IMPDLLEXPORT Optimizer
{
public:
  Optimizer();
  virtual ~Optimizer();

  /** Optimize the model
      \param[in] model The model to be optimized.
      \param[in] max_steps The maximum number of steps to take.
      \param[in] threshold Stop if the score goes below threshold.
      \return The final score.
   */       
  virtual Float optimize(Model* model, int max_steps, 
                         Float threshold=-std::numeric_limits<Float>::max())
    = 0;
  /** \return the current version*/
  virtual std::string version() const =0;
  /** \return the last person to modify this restraint */
  virtual std::string last_modified_by() const=0;
};

} // namespace IMP

#endif  /* __IMP_OPTIMIZER_H */
