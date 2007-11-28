/**
 *  \file SteepestDescent.h   \brief Simple steepest descent optimizer.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_STEEPEST_DESCENT_H
#define __IMP_STEEPEST_DESCENT_H

#include "../IMP_config.h"
#include "Optimizer.h"

namespace IMP
{

//! A simple steepest descent optimizer
 /** Follow the gradient based on the partial derivatives. Multiply by the
      current step size. If the score gets worse, reduce the step size.
      If the score gets better, increase the step size if we are sufficiently
      far from a score of zero. If the score reaches the threshold, quit.
 */
class IMPDLLEXPORT SteepestDescent : public Optimizer
{
public:
  SteepestDescent();
  virtual ~SteepestDescent();

  IMP_OPTIMIZER("0.5", "Bret Peterson");

protected:
  Float step_size;
};

} // namespace IMP

#endif  /* __IMP_STEEPEST_DESCENT_H */
