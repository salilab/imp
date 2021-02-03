/**
 *  \file IMP/core/SteepestDescent.h   \brief Simple steepest descent optimizer.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_STEEPEST_DESCENT_H
#define IMPCORE_STEEPEST_DESCENT_H

#include <IMP/core/core_config.h>

#include <IMP/AttributeOptimizer.h>

IMPCORE_BEGIN_NAMESPACE

//! A simple steepest descent optimizer
/** Follow the gradient based on the partial derivatives. Multiply by the
    current step size. If the score gets worse, reduce the step size.
    If the score gets better, increase the step size if we are sufficiently
    far from a score of zero. If the score reaches the threshold, quit.
*/
class IMPCOREEXPORT SteepestDescent : public AttributeOptimizer {
 public:
  SteepestDescent(Model *m, std::string name = "SteepestDescent%1%");

  virtual Float do_optimize(unsigned int max_steps) IMP_OVERRIDE;
  IMP_OBJECT_METHODS(SteepestDescent);

  //! Set the minimum gradient threshold
  void set_threshold(Float t) { threshold_ = t; }
  //! The starting step size
  void set_step_size(Float t) { step_size_ = t; }
  //! The maximum step size
  void set_maximum_step_size(Float t) { max_step_size_ = t; }

 private:
  Float step_size_;
  Float max_step_size_;
  Float threshold_;
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_STEEPEST_DESCENT_H */
