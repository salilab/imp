/**
 *  \file SteepestDescent.h   \brief Simple steepest descent optimizer.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_STEEPEST_DESCENT_H
#define IMPCORE_STEEPEST_DESCENT_H

#include "config.h"
#include "internal/version_info.h"

#include <IMP/Optimizer.h>

IMPCORE_BEGIN_NAMESPACE

//! A simple steepest descent optimizer
 /** Follow the gradient based on the partial derivatives. Multiply by the
     current step size. If the score gets worse, reduce the step size.
     If the score gets better, increase the step size if we are sufficiently
     far from a score of zero. If the score reaches the threshold, quit.
 */
class IMPCOREEXPORT SteepestDescent : public Optimizer
{
public:
  /** */
  SteepestDescent();
  virtual ~SteepestDescent();

  IMP_OPTIMIZER(internal::version_info)

  //! Set the minimum gradient threshold
  void set_threshold(Float t) {threshold_=t;}
  //! Set the step size
  void set_step_size(Float t) {step_size_=t;}

private:
  Float step_size_;
  Float threshold_;
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_STEEPEST_DESCENT_H */
