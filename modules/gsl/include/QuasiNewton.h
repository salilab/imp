/**
 *  \file IMP/gsl/QuasiNewton.h
 *  \brief A GSL-based Quasi-Newton optimizer
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPGSL_QUASI_NEWTON_H
#define IMPGSL_QUASI_NEWTON_H

#include <IMP/gsl/gsl_config.h>

#include "GSLOptimizer.h"

IMPGSL_BEGIN_NAMESPACE

//! A quasi-Newton optimizer taken from GSL
/** \untested{QuasiNewton}
 */
class IMPGSLEXPORT QuasiNewton : public GSLOptimizer {
  double initial_step_, line_step_, min_gradient_;

 public:
  QuasiNewton(Model *m = nullptr);

  // default 0.01
  void set_initial_step(double length) {
    IMP_USAGE_CHECK(length > 0 && length <= 4,
                    "The initial step is relative to the rescaled attributes"
                        << " and so should not be much larger than 1.");
    initial_step_ = length;
  }

  // default 0.01
  void set_line_step(double d) {
    IMP_USAGE_CHECK(d > 0 && d <= 4,
                    "The minimum size is relative to the rescaled attributes"
                        << " and so should not be much larger than 1 "
                        << "(and must be non-zero).");
    line_step_ = d;
  }

  // default 0.001
  void set_minimum_gradient(double d) {
    IMP_USAGE_CHECK(d > 0 && d <= 1, "The minimum gradient is relative.");
    min_gradient_ = d;
  }
  virtual Float do_optimize(unsigned int max_steps) IMP_OVERRIDE;
  IMP_OBJECT_METHODS(QuasiNewton);
};

IMPGSL_END_NAMESPACE

#endif /* IMPGSL_QUASI_NEWTON_H */
